#include <gtest/gtest.h>
#include "practice.hpp"

#include <atomic>
#include <chrono>
#include <future>
#include <thread>
#include <vector>

// Tests for Chapter 04: Synchronizing Concurrent Operations.

TEST(ProducerConsumerQueue, ZeroCapacityThrows) {
    EXPECT_THROW(ProducerConsumerQueue(0), std::invalid_argument);
}

TEST(ProducerConsumerQueue, PushThenPopPreservesFifoOrder) {
    ProducerConsumerQueue q(10);
    q.push(1);
    q.push(2);
    q.push(3);

    EXPECT_EQ(q.pop(), 1);
    EXPECT_EQ(q.pop(), 2);
    EXPECT_EQ(q.pop(), 3);
}

TEST(ProducerConsumerQueue, PushBlocksWhenFull) {
    ProducerConsumerQueue q(1);
    q.push(1);  // fills the queue to capacity

    std::atomic<bool> completed{false};
    std::thread producer([&q, &completed] {
        q.push(2);  // should block: queue is full
        completed = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(completed.load());  // nothing has popped yet, so this must still be blocked

    EXPECT_EQ(q.pop(), 1);  // frees a slot, unblocking the producer
    producer.join();
    EXPECT_TRUE(completed.load());
    EXPECT_EQ(q.pop(), 2);
}

TEST(ProducerConsumerQueue, PopBlocksWhenEmpty) {
    ProducerConsumerQueue q(1);

    std::atomic<bool> completed{false};
    int result = 0;
    std::thread consumer([&q, &completed, &result] {
        result = q.pop();  // should block: queue is empty
        completed = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(completed.load());  // nothing has pushed yet, so this must still be blocked

    q.push(42);  // unblocks the consumer
    consumer.join();
    EXPECT_TRUE(completed.load());
    EXPECT_EQ(result, 42);
}

TEST(ProducerConsumerQueue, ConcurrentProducersAndConsumers) {
    ProducerConsumerQueue q(4);  // small capacity forces constant blocking on both sides
    constexpr int num_producers = 4;
    constexpr int items_per_producer = 500;
    constexpr int total_items = num_producers * items_per_producer;

    std::vector<std::thread> producers;
    for (int p = 0; p < num_producers; ++p) {
        producers.emplace_back([&q, p] {
            for (int i = 0; i < items_per_producer; ++i) {
                q.push(p * items_per_producer + i);
            }
        });
    }

    std::atomic<int> consumed_count{0};
    constexpr int num_consumers = 4;
    std::vector<std::thread> consumers;
    for (int c = 0; c < num_consumers; ++c) {
        consumers.emplace_back([&q, &consumed_count] {
            for (int i = 0; i < items_per_producer; ++i) {
                q.pop();
                consumed_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto& t : producers) {
        t.join();
    }
    for (auto& t : consumers) {
        t.join();
    }

    EXPECT_EQ(consumed_count.load(), total_items);
}

TEST(ProduceItems, ReturnsSequentialRangeStartingAtZero) {
    EXPECT_EQ(produce_items(5), (std::vector<int>{0, 1, 2, 3, 4}));
}

TEST(ProduceItems, HandlesZeroCount) {
    EXPECT_TRUE(produce_items(0).empty());
}

TEST(ConsumeItems, SumsAllElements) {
    EXPECT_EQ(consume_items({1, 2, 3, 4, 5}), 15);
}

TEST(ConsumeItems, SumsEmptyToZero) {
    EXPECT_EQ(consume_items({}), 0);
}

TEST(AsyncPipeline, ComputesCorrectSum) {
    // sum of 0..999 = 999 * 1000 / 2 = 499500
    EXPECT_EQ(run_producer_consumer_pipeline_async(1000), 499500);
}

TEST(AsyncPipeline, HandlesZeroCount) {
    EXPECT_EQ(run_producer_consumer_pipeline_async(0), 0);
}

TEST(AsyncPipeline, LaunchAsyncActuallyRunsOnADifferentThread) {
    // The whole reason to pass launch::async explicitly: with no policy, the
    // implementation could pick launch::deferred instead, which would run
    // the task lazily on *this* thread inside .get() - silently sequential,
    // not concurrent at all. This proves the explicit policy actually holds.
    std::thread::id main_id = std::this_thread::get_id();
    std::future<std::thread::id> f =
        std::async(std::launch::async, [] { return std::this_thread::get_id(); });

    EXPECT_NE(f.get(), main_id);
}
