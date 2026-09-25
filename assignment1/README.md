# CSC 718 – Assignment 1

**Student:** Sanjeev Kumar Khatri  
**Language:** C++  
**Threading API:** POSIX Threads (`pthread`)

## Project Structure

```text
assignment1/
├── README.md
├── r1/
│   ├── queue.hpp
│   ├── queue.cpp
│   └── r1_test.cpp
├── r2/
│   ├── queue.hpp
│   ├── queue.cpp
│   ├── pool.hpp
│   ├── pool.cpp
│   └── r2_main.cpp
└── r3/
    └── r3_results.pdf
```

## R1 – Bounded Queue

R1 implements a thread-safe bounded queue using exactly one mutex and two
condition variables (`not_full` and `not_empty`). The test uses multiple
producers and consumers and verifies that every submitted job is processed
exactly once.

### Build

```bash
cd r1
g++ -std=c++17 -Wall -Wextra -Wpedantic -pthread     queue.cpp r1_test.cpp -o r1_test
```

### Run

```bash
./r1_test
```

R1 test configuration:

```text
Producers:       4
Consumers:       4
Queue capacity:  10
Total jobs:      1000
Expected ID sum: 500500
```

---

## R2 – Worker Pool

R2 builds a fixed-size worker pool using the bounded queue from R1.

### Build

```bash
cd r2
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -pthread     queue.cpp pool.cpp r2_main.cpp -o r2_main
```

### Run

The worker count `N` is provided as a command-line argument:

```bash
./r2_main N
```

Examples:

```bash
./r2_main 1
./r2_main 2
./r2_main 4
./r2_main 8
```

### Workload Parameters

```text
Worker count (N): command-line argument
Number of jobs (M): 100
Queue capacity: 10
Job type: prime counting
Prime limit per job: 400000
```

Each job counts primes from 2 through 400000. The same deterministic workload
is used for every job.

The original prime limit was 150000, but the 1-worker runtime was only about
0.93 seconds. The limit was increased to 400000 so the 1-worker run would take
at least a few seconds for more meaningful performance measurements.

### Timing Method

Timing uses:

```cpp
clock_gettime(CLOCK_MONOTONIC, ...)
```

The timer starts after the worker threads are created and immediately before
job submission. It stops after all jobs have completed and before worker-pool
shutdown.

Worker creation and shutdown are therefore excluded from the measured time.

### Shutdown

Workers are shut down using poison-pill jobs. One poison pill is submitted for
each worker, and the main thread joins all workers with `pthread_join()`.

---

## R3 – Scaling Study

The scaling study runs the same program with:

```bash
./r2_main 1
./r2_main 2
./r2_main 4
./r2_main 8
```

The same number of jobs, job workload, queue capacity, timing method, compiler
settings, and GitHub Codespaces environment are used for all runs.

The complete results, speedup calculations, screenshots, and discussion are in:

```text
r3/r3_results.pdf
```

## Assumptions and Known Limitations

- The workload is deterministic and uses the same prime limit for every job.
- The GitHub Codespaces environment exposed only 2 logical CPUs.
- The 4-worker and 8-worker configurations therefore oversubscribe the
  available CPU resources.
- Queue synchronization, mutex/condition-variable operations, thread
  scheduling, and context switching may limit scaling.
- Queue capacity is fixed at 10.

## AI Use Disclosure

I have used AI for function definitions and function naming. I have also used it
testing and debugging the code.
