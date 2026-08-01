#include <gtest/gtest.h>
#include "practice.hpp"

#include <atomic>
#include <chrono>
#include <numeric>
#include <type_traits>

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

// scoped_thread: non-copyable, but move-only (checked at compile time).
static_assert(!std::is_copy_constructible_v<scoped_thread>);
static_assert(!std::is_copy_assignable_v<scoped_thread>);
static_assert(std::is_move_constructible_v<scoped_thread>);
static_assert(std::is_move_assignable_v<scoped_thread>);

TEST(ScopedThread, JoinsInDestructor) {
    std::atomic<bool> finished{false};
    {
        scoped_thread st(std::thread([&finished] {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            finished = true;
        }));
    }  // destructor must block here until the thread actually completes

    EXPECT_TRUE(finished.load());
}

TEST(ScopedThread, ThrowsOnNonJoinableThread) {
    EXPECT_THROW(scoped_thread bad(std::thread{}), std::logic_error);
}

TEST(ScopedThread, DoesNotThrowOnJoinableThread) {
    EXPECT_NO_THROW(scoped_thread good(std::thread([] {})));
}

TEST(ScopedThread, MoveConstructionTransfersOwnership) {
    std::atomic<bool> finished{false};
    {
        scoped_thread a(std::thread([&finished] {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            finished = true;
        }));
        scoped_thread b(std::move(a));
        // `a` is now moved-from (non-joinable); its destructor at scope exit
        // must be a safe no-op, and `b` must be the one that actually joins.
    }

    EXPECT_TRUE(finished.load());
}

TEST(ScopedThread, MoveAssignmentJoinsExistingThreadFirst) {
    std::atomic<bool> first_finished{false};
    std::atomic<bool> second_finished{false};
    {
        scoped_thread a(std::thread([&first_finished] {
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            first_finished = true;
        }));
        scoped_thread b(std::thread([&second_finished] {
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            second_finished = true;
        }));

        a = std::move(b);
        // `a` owned a joinable thread before this assignment; if the
        // assignment didn't join it first, std::thread::operator= would
        // have called std::terminate() instead of getting here.
        EXPECT_TRUE(first_finished.load());
    }  // `a` (now holding what was `b`'s thread) joins here; `b` is moved-from

    EXPECT_TRUE(second_finished.load());
}

TEST(ScopedThread, SelfMoveAssignmentIsSafe) {
    std::atomic<bool> finished{false};
    {
        scoped_thread a(std::thread([&finished] {
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            finished = true;
        }));
        auto& self_ref = a;
        a = std::move(self_ref);
    }

    EXPECT_TRUE(finished.load());
}
