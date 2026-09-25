#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <pthread.h>
#include <cstddef>
#include <vector>

class BoundedQueue {
private:
    std::vector<int> buffer;

    std::size_t capacity;
    std::size_t head;
    std::size_t tail;
    std::size_t count;

    // Exactly one mutex protects the queue state.
    pthread_mutex_t mutex;

    // Producer waits when queue is full.
    pthread_cond_t not_full;

    // Consumer waits when queue is empty.
    pthread_cond_t not_empty;

public:
    explicit BoundedQueue(std::size_t capacity);
    ~BoundedQueue();

    // Adding an item to the queue.
    // Blocking if the queue is full.
    void enqueue(int value);

    // Remove an item from the queue.
    // Blocks if the queue is empty.
    int dequeue();

    // we wont allow copying because pthread synchronize and objects should not be copied.
    BoundedQueue(const BoundedQueue&) = delete;
    BoundedQueue& operator=(const BoundedQueue&) = delete;
};

#endif