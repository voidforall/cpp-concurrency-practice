#include <gtest/gtest.h>
#include "practice.hpp"

#include <atomic>
#include <chrono>
#include <numeric>
#include <stop_token>
#include <thread>
#include <vector>

// Tests for Chapter 02: Managing Threads.

TEST(ParallelFor, MutatesEveryElementAcrossManyChunks) {
    std::vector<int> v(1000);
    std::iota(v.begin(), v.end(), 1);  // 1..1000

    parallel_for(v.begin(), v.end(), [](int& x) { x *= 2; });

    for (int i = 0; i < 1000; ++i) {
        EXPECT_EQ(v[i], (i + 1) * 2);
    }
}

TEST(ParallelFor, HandlesEmptyRangeAsNoOp) {
    std::vector<int> empty;

    EXPECT_NO_THROW(parallel_for(empty.begin(), empty.end(), [](int& x) { x *= 2; }));
}

TEST(ParallelFor, HandlesFewerElementsThanHardwareThreads) {
    std::vector<int> small = {1, 2, 3};

    parallel_for(small.begin(), small.end(), [](int& x) { x *= 10; });

    EXPECT_EQ(small[0], 10);
    EXPECT_EQ(small[1], 20);
    EXPECT_EQ(small[2], 30);
}

TEST(ParallelFor, SafeConcurrentAccumulationViaAtomic) {
    std::vector<int> v(100);
    std::iota(v.begin(), v.end(), 1);  // 1..100
    std::atomic<int> total{0};

    parallel_for(v.begin(), v.end(), [&total](int x) { total += x; });

    EXPECT_EQ(total.load(), std::accumulate(v.begin(), v.end(), 0));
}

TEST(ParallelForCancellable, ProcessesEverythingWhenNeverCancelled) {
    std::vector<int> v(100, 0);
    std::stop_source source;  // never requests stop

    parallel_for(v.begin(), v.end(), [](int& x) { x = 1; }, source.get_token());

    for (int x : v) {
        EXPECT_EQ(x, 1);
    }
}

TEST(ParallelForCancellable, StopsEarlyWhenCancelledMidway) {
    std::vector<int> v(1000, 0);
    std::stop_source source;
    std::atomic<int> processed_count{0};

    // parallel_for(..., stop_token) blocks until its workers finish or bail,
    // so the only way to actually cancel it mid-flight is from another
    // thread while it's running.
    std::thread canceller([&source] {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        source.request_stop();
    });

    parallel_for(v.begin(), v.end(), [&](int& x) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        x = 1;
        processed_count.fetch_add(1, std::memory_order_relaxed);
    }, source.get_token());

    canceller.join();

    // With a 1ms delay per element and cancellation requested at 50ms,
    // each worker should get through roughly ~50 elements before observing
    // stop_requested() and bailing - nowhere near all 1000, but definitely
    // more than 0.
    EXPECT_GT(processed_count.load(), 0);
    EXPECT_LT(processed_count.load(), 1000);
}
