#include "queue.hpp"

#include <stdexcept>

BoundedQueue::BoundedQueue(std::size_t capacity)
    : buffer(capacity),
      capacity(capacity),
      head(0),
      tail(0),
      count(0)
{
    if (capacity == 0) {
        throw std::invalid_argument(
            "Queue capacity must be greater than zero"
        );
    }

    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&not_full, nullptr);
    pthread_cond_init(&not_empty, nullptr);
}

BoundedQueue::~BoundedQueue()
{
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);
    pthread_mutex_destroy(&mutex);
}

void BoundedQueue::enqueue(int value)
{
    pthread_mutex_lock(&mutex);

    // If the queue is full, sleep until a consumer removes an item and signals not_full.
    //  i am using while instead of if because the conditon must be checked after waking.
    while (count == capacity) {
        pthread_cond_wait(&not_full, &mutex);
    }

    // Inserting at the tail
    buffer[tail] = value;

    // Circular-buffer wraparound 
    tail = (tail + 1) % capacity;

    count++;

    // The queue is no longer empty so, Waking one consumer if one is waiting
    pthread_cond_signal(&not_empty);

    pthread_mutex_unlock(&mutex);
}

int BoundedQueue::dequeue()
{
    pthread_mutex_lock(&mutex);

    // If the queue is empty, this will sleep until a producer adds an item and signals not_empty
    while (count == 0) {
        pthread_cond_wait(&not_empty, &mutex);
    }

    // Remove item from the head.
    int value = buffer[head];

    // Circular-buffer wraparound.
    head = (head + 1) % capacity;

    count--;

    //  The queue now has free space, Waking one producer if one is waiting
    pthread_cond_signal(&not_full);

    pthread_mutex_unlock(&mutex);

    return value;
}