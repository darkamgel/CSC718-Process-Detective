#ifndef POOL_HPP
#define POOL_HPP

#include "queue.hpp"

#include <pthread.h>
#include <cstddef>
#include <vector>

class WorkerPool {
private:
    // Bounded queue from R1.
    BoundedQueue job_queue;

    // Worker thread IDs.
    std::vector<pthread_t> workers;

    // Number of worker threads in pool.
    std::size_t worker_count;

    // Synchronization for tracking completed jobs.
    pthread_mutex_t completion_mutex;
    pthread_cond_t all_done;

    // Number of real jobs submitted and completed.
    std::size_t submitted_jobs;
    std::size_t completed_jobs;

    // Workload used by every job.
    int prime_limit;

    // Tracking whether workers are currently running.
    bool started;
    bool shutdown_complete;

    // Special job value used to terminate workers.
    static constexpr int POISON_PILL = -1;
    static void *worker_entry(void *arg);

    // Main worker loop.
    void worker_loop();

    // Execute one computational job.
    void execute_job(int job_id);

    // Deterministic computation used for every job.
    static int count_primes(int limit);

public:
    WorkerPool(
        std::size_t number_of_workers,
        std::size_t queue_capacity,
        int prime_limit
    );

    ~WorkerPool();

    // Creating all worker threads.
    bool start();

    // Submiting one real job to the bounded queue.
    void submit(int job_id);

    // Waiting until every submitted real job has completed.
    void wait_for_all();

    // Terminate and join all worker threads cleanly.
    void shutdown();

    // Used when printing the R2 execution summary.
    std::size_t get_completed_jobs();
};

#endif