#include "pool.hpp"

#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <iostream>


// Workload configuration
// Number of jobs submitted in every experiment.
const int NUM_JOBS = 100;

// Capacity of the bounded queue from R1.
const int QUEUE_CAPACITY = 10;

// Every job counts primes from 2 through this value.
//
// This is our INITIAL workload.
// After confirming R2 works, we will test the 1-worker
// runtime and adjust this value if necessary so that
// the execution takes at least a few seconds.
// const int PRIME_LIMIT = 150000 => this is later changed
const int PRIME_LIMIT = 400000;


// Calculate elapsed time
double elapsed_seconds(
    const timespec &start,
    const timespec &end
)
{
    double seconds =
        static_cast<double>(
            end.tv_sec - start.tv_sec
        );

    double nanoseconds =
        static_cast<double>(
            end.tv_nsec - start.tv_nsec
        ) / 1e9;

    return seconds + nanoseconds;
}



// Main
int main(int argc, char *argv[])
{
    if (argc != 2) {

        std::cerr
            << "Usage: "
            << argv[0]
            << " <number_of_workers>\n";

        return EXIT_FAILURE;
    }


    // Convert command-line argument to integer.
    int worker_count = std::atoi(argv[1]);

    if (worker_count <= 0) {

        std::cerr
            << "Error: worker count must be "
            << "greater than zero.\n";

        return EXIT_FAILURE;
    }

    std::cout<< "R2 Worker Pool\n";

    WorkerPool pool(
        static_cast<std::size_t>(worker_count),
        QUEUE_CAPACITY,
        PRIME_LIMIT
    );


    // Create worker threads
    if (!pool.start()) {

        std::cerr<< "Error: worker pool could not start.\n";
        return EXIT_FAILURE;
    }

    timespec start_time;
    timespec end_time;

    clock_gettime(
        CLOCK_MONOTONIC,
        &start_time
    );


    // Submit jobs
    for (int job_id = 1;
         job_id <= NUM_JOBS;
         job_id++) {

        pool.submit(job_id);
    }

    // Wait until ALL real jobs have completed
    pool.wait_for_all();
    clock_gettime(
        CLOCK_MONOTONIC,
        &end_time
    );

    // Calculate performance
    double elapsed =
        elapsed_seconds(
            start_time,
            end_time
        );

    std::size_t completed =
        pool.get_completed_jobs();


    double throughput = 0.0;

    if (elapsed > 0.0) {

        throughput =
            static_cast<double>(completed)
            / elapsed;
    }
    pool.shutdown();


    // Print required R2 summary
    std::cout<< "Workers:"<< worker_count<< "\n";
    std::cout<< "Jobs submitted:"<< NUM_JOBS<< "\n";
    std::cout<< "Jobs processed:"<< completed<< "\n";
    std::cout<< "Prime limit/job:"<< PRIME_LIMIT<< "\n";
    std::cout<< "Queue capacity:"<< QUEUE_CAPACITY<< "\n";
    std::cout<< std::fixed<< std::setprecision(4);
    std::cout<< "Total elapsed time:"<< elapsed<< " seconds\n";
    std::cout<< "Throughput:"<< throughput<< " jobs/second\n";

    // Final correctness check
    if (completed != NUM_JOBS) {

        std::cout
            << "\nRESULT: FAIL\n";

        std::cout
            << "Not all submitted jobs were completed.\n";

        return EXIT_FAILURE;
    }


    std::cout
        << "\nRESULT: PASS\n";

    std::cout
        << "All jobs completed and workers "
        << "shut down cleanly.\n";


    return EXIT_SUCCESS;
}