#include <gtest/gtest.h>
#include "practice.hpp"

#include <numeric>

// Tests for Chapter 01: Hello, World of Concurrency in C++!.

TEST(CalculatePartialSum, TotalMatchesSequentialSum) {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<int> partial_sums;

    calculate_partial_sum(numbers, 3, partial_sums);

    int total = std::accumulate(partial_sums.begin(), partial_sums.end(), 0);
    EXPECT_EQ(total, std::accumulate(numbers.begin(), numbers.end(), 0));
}

TEST(CalculatePartialSum, ChunkBoundariesMatchExpectedSplit) {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<int> partial_sums;

    calculate_partial_sum(numbers, 3, partial_sums);

    // chunk_size = 10 / 3 = 3: [0,3) [3,6) [6,10) (last thread absorbs the remainder)
    ASSERT_EQ(partial_sums.size(), 3u);
    EXPECT_EQ(partial_sums[0], 1 + 2 + 3);
    EXPECT_EQ(partial_sums[1], 4 + 5 + 6);
    EXPECT_EQ(partial_sums[2], 7 + 8 + 9 + 10);
}

TEST(CalculatePartialSum, SingleThreadSumsEverything) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    std::vector<int> partial_sums;

    calculate_partial_sum(numbers, 1, partial_sums);

    ASSERT_EQ(partial_sums.size(), 1u);
    EXPECT_EQ(partial_sums[0], 15);
}
