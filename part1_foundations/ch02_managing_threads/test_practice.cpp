#include <gtest/gtest.h>
#include "practice.hpp"

#include <atomic>
#include <numeric>
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
