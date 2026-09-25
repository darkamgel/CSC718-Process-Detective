#include "pool.hpp"

#include <iostream>
#include <stdexcept>

// Constructor
WorkerPool::WorkerPool(
    std::size_t number_of_workers,
    std::size_t queue_capacity,
    int prime_limit)
    : job_queue(queue_capacity),
      workers(number_of_workers),
      worker_count(number_of_workers),
      submitted_jobs(0),
      completed_jobs(0),
      prime_limit(prime_limit),
      started(false),
      shutdown_complete(false)
{
    if (number_of_workers == 0)
    {
        throw std::invalid_argument(
            "Worker count must be greater than zero");
    }

    if (prime_limit < 2)
    {
        throw std::invalid_argument(
            "Prime limit must be at least 2");
    }

    int rc = pthread_mutex_init(
        &completion_mutex,
        nullptr);

    if (rc != 0)
    {
        throw std::runtime_error(
            "Failed to initialize completion mutex");
    }

    rc = pthread_cond_init(
        &all_done,
        nullptr);

    if (rc != 0)
    {
        pthread_mutex_destroy(&completion_mutex);

        throw std::runtime_error(
            "Failed to initialize completion condition variable");
    }
}

// Destructor
WorkerPool::~WorkerPool()
{
    if (started && !shutdown_complete)
    {
        shutdown();
    }

    pthread_cond_destroy(&all_done);
    pthread_mutex_destroy(&completion_mutex);
}

// pthread entry function
void *WorkerPool::worker_entry(void *arg)
{
    WorkerPool *pool =
        static_cast<WorkerPool *>(arg);

    pool->worker_loop();

    return nullptr;
}

// Worker loop
void WorkerPool::worker_loop()
{
    while (true)
    {
        int job_id = job_queue.dequeue();
        if (job_id == POISON_PILL)
        {
            break;
        }

        execute_job(job_id);
    }
}

// Executing one job
void WorkerPool::execute_job(int job_id)
{
    volatile int result = count_primes(prime_limit);
    (void)result;
    (void)job_id;

    pthread_mutex_lock(&completion_mutex);

    completed_jobs++;

    if (completed_jobs == submitted_jobs)
    {
        pthread_cond_signal(&all_done);
    }

    pthread_mutex_unlock(&completion_mutex);
}

// Prime-counting workload
int WorkerPool::count_primes(int limit)
{
    int prime_count = 0;

    for (int number = 2;
         number <= limit;
         number++)
    {

        bool is_prime = true;
        for (int divisor = 2;
             divisor <= number / divisor;
             divisor++)
        {

            if (number % divisor == 0)
            {
                is_prime = false;
                break;
            }
        }

        if (is_prime)
        {
            prime_count++;
        }
    }

    return prime_count;
}

// Starting worker threads
bool WorkerPool::start()
{
    if (started)
    {
        return false;
    }

    std::size_t created = 0;

    for (std::size_t i = 0;
         i < worker_count;
         i++)
    {

        int rc = pthread_create(
            &workers[i],
            nullptr,
            worker_entry,
            this);

        if (rc != 0)
        {

            std::cerr << "Failed to create worker " << i << ". pthread_create returned " << rc << "\n";

            for (std::size_t j = 0;
                 j < created;
                 j++)
            {

                job_queue.enqueue(POISON_PILL);
            }

            for (std::size_t j = 0;
                 j < created;
                 j++)
            {

                pthread_join(
                    workers[j],
                    nullptr);
            }

            return false;
        }

        created++;
    }

    started = true;

    return true;
}

// Submiting a job
void WorkerPool::submit(int job_id)
{
    if (!started || shutdown_complete)
    {
        throw std::runtime_error(
            "Cannot submit job to inactive worker pool");
    }

    if (job_id == POISON_PILL)
    {
        throw std::invalid_argument(
            "Job ID cannot use the poison-pill value");
    }

    pthread_mutex_lock(&completion_mutex);

    submitted_jobs++;

    pthread_mutex_unlock(&completion_mutex);

    job_queue.enqueue(job_id);
}

// Waiting until all submitted jobs are complete
void WorkerPool::wait_for_all()
{
    pthread_mutex_lock(&completion_mutex);

    while (completed_jobs < submitted_jobs)
    {

        pthread_cond_wait(
            &all_done,
            &completion_mutex);
    }

    pthread_mutex_unlock(&completion_mutex);
}

// Shutdown
void WorkerPool::shutdown()
{
    if (!started || shutdown_complete)
    {
        return;
    }

    for (std::size_t i = 0;
         i < worker_count;
         i++)
    {
        job_queue.enqueue(POISON_PILL);
    }

    for (std::size_t i = 0;
         i < worker_count;
         i++)
    {
        pthread_join(
            workers[i],
            nullptr);
    }

    shutdown_complete = true;
}

// Return number of completed jobs
std::size_t WorkerPool::get_completed_jobs()
{
    pthread_mutex_lock(&completion_mutex);
    std::size_t result = completed_jobs;
    pthread_mutex_unlock(&completion_mutex);

    return result;
}