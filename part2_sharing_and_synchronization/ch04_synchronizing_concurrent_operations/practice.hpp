// Chapter 04: Synchronizing Concurrent Operations — practice.
//
// Mini-project: Build a bounded producer-consumer queue using a mutex + condition_variable (predicate-based wait for both 'not full' and 'not empty'), then re-implement the same producer/consumer pipeline using `std::async` and compare the two designs.
//
// Declare your types/functions here; implement in practice.cpp.

#ifndef CH04_SYNCHRONIZING_CONCURRENT_OPERATIONS_PRACTICE_HPP
#define CH04_SYNCHRONIZING_CONCURRENT_OPERATIONS_PRACTICE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <future>
#include <vector>
#include <numeric>

class ProducerConsumerQueue {
public:
    explicit ProducerConsumerQueue(size_t capacity) : capacity_(capacity) {
        if (capacity_ == 0) {
            throw std::invalid_argument("ProducerConsumerQueue capacity must be > 0");
        }
    }

    void push(int value) {
        std::unique_lock<std::mutex> lock(mutex_);

        not_full_.wait(lock, [this] { return queue_.size() < capacity_; }); // block until the queue is not full
        queue_.push(value);

        lock.unlock();  // release before notifying so the woken thread doesn't
                         // immediately block trying to re-acquire this mutex
        not_empty_.notify_one();
    }

    int pop() {
        std::unique_lock<std::mutex> lock(mutex_);

        not_empty_.wait(lock, [this] { return !queue_.empty(); }); // block until the queue is not empty
        int value = queue_.front();
        queue_.pop();

        lock.unlock();
        not_full_.notify_one();

        return value;
    }

private:
    std::queue<int> queue_;
    std::mutex mutex_;
    std::condition_variable not_full_;
    std::condition_variable not_empty_;
    size_t capacity_;
};

// std::async re-implementation of the same producer/consumer idea.
//
// This is NOT a like-for-like swap of primitives - std::future is a one-shot
// "run this, get one result back later" handle, not a live, shared, bounded
// channel multiple threads can push/pop through over time. So the pipeline's
// shape has to change: the "producer" runs to completion and hands back its
// whole batch at once (no backpressure, no streaming), and the "consumer"
// then processes that batch as a second async stage. Comparing this against
// ProducerConsumerQueue is the actual point of the kata.

inline std::vector<int> produce_items(int count) {
    std::vector<int> items;
    items.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        items.push_back(i);
    }
    return items;
}

inline long long consume_items(std::vector<int> items) {
    return std::accumulate(items.begin(), items.end(), 0LL);
}

inline long long run_producer_consumer_pipeline_async(int count) {
    // launch::async is explicit on purpose: the default (no policy) lets the
    // implementation choose launch::deferred instead, which would run
    // produce_items() lazily on *this* thread inside .get() rather than
    // concurrently - silently turning "async" into "sequential."
    std::future<std::vector<int>> produced =
        std::async(std::launch::async, produce_items, count);

    std::vector<int> items = produced.get();  // blocks until the producer stage is done

    std::future<long long> consumed =
        std::async(std::launch::async, consume_items, std::move(items));

    return consumed.get();  // blocks until the consumer stage is done
}

#endif // CH04_SYNCHRONIZING_CONCURRENT_OPERATIONS_PRACTICE_HPP
