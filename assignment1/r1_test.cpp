#include "queue.hpp"

#include <pthread.h>
#include <iostream>
#include <vector>

// Test configuration
const int NUM_PRODUCERS = 4;
const int NUM_CONSUMERS = 4;

const int JOBS_PER_PRODUCER = 250;

const int TOTAL_JOBS =
    NUM_PRODUCERS * JOBS_PER_PRODUCER;

const int QUEUE_CAPACITY = 10;

// Special value used to tell a consumer to terminate.
const int POISON_PILL = -1;



// Produceing thread arguments
struct ProducerArgs {
    BoundedQueue *queue;
    int start_id;
    int end_id;
};


// Consumers thread arguments
struct ConsumerArgs {
    BoundedQueue *queue;

    /*
     * Each consumer owns its own vector.
     *
     * Therefore, no additional mutex is needed for
     * recording processed jobs.
     */
    std::vector<int> processed_jobs;
};


// Producer thread
void *producer_function(void *arg)
{
    ProducerArgs *data =
        static_cast<ProducerArgs *>(arg);

    /*
     * Submit every job in this producer's assigned range.
     */
    for (int job_id = data->start_id;
         job_id <= data->end_id;
         job_id++) {

        data->queue->enqueue(job_id);
    }

    return nullptr;
}


// Consumer thread
void *consumer_function(void *arg)
{
    ConsumerArgs *data =
        static_cast<ConsumerArgs *>(arg);

    while (true) {
        int job_id = data->queue->dequeue();
        if (job_id == POISON_PILL) {
            break;
        }
        data->processed_jobs.push_back(job_id);
    }

    return nullptr;
}

// Main test
int main()
{
    std::cout<< "R1 Bounded Queue Test\n";

    // Create the bounded queue.
    BoundedQueue queue(QUEUE_CAPACITY);

    // pthread identifiers.
    pthread_t producer_threads[NUM_PRODUCERS];
    pthread_t consumer_threads[NUM_CONSUMERS];

    // Arguments passed to each thread.
    ProducerArgs producer_args[NUM_PRODUCERS];
    ConsumerArgs consumer_args[NUM_CONSUMERS];

    // Starting consumers
    for (int i = 0; i < NUM_CONSUMERS; i++) {

        consumer_args[i].queue = &queue;

        int rc = pthread_create(
            &consumer_threads[i],
            nullptr,
            consumer_function,
            &consumer_args[i]
        );

        if (rc != 0) {
            std::cerr
                << "Error: failed to create consumer "
                << i
                << ". pthread_create returned "
                << rc
                << "\n";

            return 1;
        }
    }


    // Starting producers
    for (int i = 0; i < NUM_PRODUCERS; i++) {

        producer_args[i].queue = &queue;

        producer_args[i].start_id =
            (i * JOBS_PER_PRODUCER) + 1;

        producer_args[i].end_id =
            (i + 1) * JOBS_PER_PRODUCER;

        int rc = pthread_create(
            &producer_threads[i],
            nullptr,
            producer_function,
            &producer_args[i]
        );

        if (rc != 0) {
            std::cerr
                << "Error: failed to create producer "
                << i
                << ". pthread_create returned "
                << rc
                << "\n";

            return 1;
        }
    }


    // Wait for producers
    for (int i = 0; i < NUM_PRODUCERS; i++) {

        int rc = pthread_join(
            producer_threads[i],
            nullptr
        );

        if (rc != 0) {
            std::cerr
                << "Error joining producer "
                << i
                << "\n";

            return 1;
        }
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        queue.enqueue(POISON_PILL);
    }


    // Wait for consumers
    for (int i = 0; i < NUM_CONSUMERS; i++) {

        int rc = pthread_join(
            consumer_threads[i],
            nullptr
        );

        if (rc != 0) {
            std::cerr
                << "Error joining consumer "
                << i
                << "\n";

            return 1;
        }
    }


    // Correctness verification
    int total_processed = 0;

    long long observed_sum = 0;
    std::vector<int> seen(
        TOTAL_JOBS + 1,
        0
    );

    bool valid_job_ids = true;

    for (int i = 0; i < NUM_CONSUMERS; i++) {

        for (int job_id :
             consumer_args[i].processed_jobs) {

            total_processed++;

            observed_sum += job_id;

            if (job_id >= 1 &&
                job_id <= TOTAL_JOBS) {

                seen[job_id]++;

            } else {

                valid_job_ids = false;

                std::cout
                    << "Invalid job ID detected: "
                    << job_id
                    << "\n";
            }
        }
    }


    // Calculate expected sum
    long long expected_sum =
        static_cast<long long>(TOTAL_JOBS) *
        (TOTAL_JOBS + 1) / 2;


    // Check exactly-once execution
    bool exactly_once = true;

    for (int job_id = 1;
         job_id <= TOTAL_JOBS;
         job_id++) {

        if (seen[job_id] != 1) {

            exactly_once = false;

            std::cout
                << "Job "
                << job_id
                << " was processed "
                << seen[job_id]
                << " times.\n";
        }
    }


    // Print results
    std::cout << "\n";
    std::cout<< "Producers:"<< NUM_PRODUCERS<< "\n";
    std::cout << "Consumers:"<< NUM_CONSUMERS<< "\n";
    std::cout << "Queue capacity:"<< QUEUE_CAPACITY<< "\n";
    std::cout << "Jobs submitted:"<< TOTAL_JOBS<< "\n";
    std::cout<< "Jobs processed:"<< total_processed<< "\n";
    std::cout<< "Expected ID sum: "<< expected_sum<< "\n";
    std::cout << "Observed ID sum:"<< observed_sum<< "\n";


    // Final PASS / FAIL
    bool passed =
        total_processed == TOTAL_JOBS &&
        observed_sum == expected_sum &&
        exactly_once &&
        valid_job_ids;


    std::cout << "\n";

    if (passed) {
        std::cout<< "RESULT: PASS\n";
        std::cout<< "Every submitted job was processed "<< "exactly once.\n";
        return 0;
    }
    std::cout<< "RESULT: FAIL\n";
    std::cout<< "The bounded queue correctness test "<< "detected an error.\n";

    return 1;
}