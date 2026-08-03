#include <gtest/gtest.h>
#include "practice.hpp"

#include <atomic>
#include <thread>
#include <vector>

// Tests for Chapter 03: Sharing Data Between Threads.

TEST(ThreadSafeStack, EmptyOnConstruction) {
    ThreadSafeStack<int> stack;
    EXPECT_TRUE(stack.empty());
}

TEST(ThreadSafeStack, PopOnEmptyReturnsNullptr) {
    ThreadSafeStack<int> stack;
    EXPECT_EQ(stack.pop(), nullptr);
}

TEST(ThreadSafeStack, PushThenPopPreservesLifoOrder) {
    ThreadSafeStack<int> stack;
    stack.push(1);
    stack.push(2);
    stack.push(3);

    EXPECT_FALSE(stack.empty());
    EXPECT_EQ(*stack.pop(), 3);
    EXPECT_EQ(*stack.pop(), 2);
    EXPECT_EQ(*stack.pop(), 1);
    EXPECT_TRUE(stack.empty());
    EXPECT_EQ(stack.pop(), nullptr);
}

TEST(ThreadSafeStack, ConcurrentPushFromMultipleProducers) {
    ThreadSafeStack<int> stack;
    constexpr int num_producers = 8;
    constexpr int items_per_producer = 1000;
    constexpr int total_items = num_producers * items_per_producer;

    std::vector<std::thread> producers;
    for (int p = 0; p < num_producers; ++p) {
        producers.emplace_back([&stack, p] {
            for (int i = 0; i < items_per_producer; ++i) {
                stack.push(p * items_per_producer + i);
            }
        });
    }
    for (auto& t : producers) {
        t.join();
    }

    // All pushes are individually mutex-protected, so none should be lost
    // regardless of contention between producers.
    int count = 0;
    while (stack.pop()) {
        ++count;
    }
    EXPECT_EQ(count, total_items);
}

TEST(ThreadSafeStack, ConcurrentPopFromMultipleConsumersDrainsExactlyOnce) {
    ThreadSafeStack<int> stack;
    constexpr int total_items = 5000;
    for (int i = 0; i < total_items; ++i) {
        stack.push(i);
    }

    // The whole point of combining "check empty + read + remove" into one
    // locked pop() is that multiple consumers can drain concurrently without
    // any two of them ever getting the same element or popping past empty.
    std::atomic<int> popped_count{0};
    constexpr int num_consumers = 8;
    std::vector<std::thread> consumers;
    for (int c = 0; c < num_consumers; ++c) {
        consumers.emplace_back([&stack, &popped_count] {
            while (stack.pop()) {
                popped_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    for (auto& t : consumers) {
        t.join();
    }

    EXPECT_EQ(popped_count.load(), total_items);
    EXPECT_TRUE(stack.empty());
}

TEST(LazySingletonCallOnce, ConcurrentCallersGetSameFullyConstructedInstance) {
    constexpr int num_threads = 32;
    std::vector<LazySingleton*> results(num_threads);
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&results, i] {
            results[i] = &get_instance_call_once();
        });
    }
    for (auto& t : threads) {
        t.join();
    }

    // Every caller must get the exact same instance...
    for (int i = 0; i < num_threads; ++i) {
        EXPECT_EQ(results[i], results[0]);
    }
    // ...and it must be fully constructed, never a half-built object -
    // this is precisely the guarantee call_once gives that the naive
    // double-checked-locking version does not.
    EXPECT_EQ(results[0]->value(), 42);
}
