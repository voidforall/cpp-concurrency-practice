#include <gtest/gtest.h>
#include "practice.hpp"

#include <array>
#include <atomic>
#include <thread>
#include <vector>

// Tests for Chapter 05: The C++ Memory Model and Operations on Atomic Types.

namespace {

template <typename Lock>
void expect_mutual_exclusion_under_contention() {
    Lock lock;
    long long counter = 0;
    constexpr int num_threads = 8;
    constexpr int iters = 100'000;

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&] {
            for (int j = 0; j < iters; ++j) {
                std::lock_guard<Lock> guard(lock);
                ++counter;
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(counter, static_cast<long long>(num_threads) * iters);
}

constexpr int kNumOwners = 16;

struct RefCountedResource : IntrusiveRefCounted {
    std::array<int, kNumOwners> slots{};
    static inline std::atomic<int> destroy_count{0};

    ~RefCountedResource() {
        destroy_count.fetch_add(1, std::memory_order_relaxed);
    }
};

}  // namespace

TEST(Spinlock, MutualExclusionUnderContention) {
    expect_mutual_exclusion_under_contention<Spinlock>();
}

TEST(SpinlockTTAS, MutualExclusionUnderContention) {
    expect_mutual_exclusion_under_contention<SpinlockTTAS>();
}

TEST(SpscHandoff, AcquireReleaseAlwaysCorrect) {
    constexpr int trials = 2000;
    for (int i = 0; i < trials; ++i) {
        SpscHandoff h;
        std::thread producer([&h, i] { h.produce(i); });
        int result = h.consume();
        producer.join();
        EXPECT_EQ(result, i);
    }
}

TEST(IntrusiveRefCounted, SingleThreadedAddRefRelease) {
    struct Counted : IntrusiveRefCounted {};
    auto* obj = new Counted();
    EXPECT_EQ(obj->use_count(), 1);

    obj->add_ref();
    EXPECT_EQ(obj->use_count(), 2);

    EXPECT_FALSE(obj->release());  // count 2 -> 1, not the last reference
    EXPECT_EQ(obj->use_count(), 1);

    EXPECT_TRUE(obj->release());  // count 1 -> 0, this is the last reference
    delete obj;
}

TEST(IntrusiveRefCounted, ConcurrentReleaseDestroysExactlyOnceAndSeesAllWrites) {
    RefCountedResource::destroy_count = 0;

    auto* res = new RefCountedResource();  // count = 1 (this test's own reference)
    for (int i = 0; i < kNumOwners; ++i) {
        res->add_ref();  // count = 1 + kNumOwners
    }

    std::atomic<bool> destroyed{false};
    std::atomic<int> observed_sum{-1};

    // The payload check happens inside the winning thread's own lambda,
    // before this test ever joins anyone - so it can't accidentally
    // piggyback on std::thread::join()'s own happens-before guarantee.
    // Whichever release() call drops the count to zero must see every
    // other owner thread's write to its own slot through the ref
    // counter's own acquire/release synchronization alone.
    auto try_release_and_verify = [&](RefCountedResource* r) {
        if (r->release()) {
            int sum = 0;
            for (int v : r->slots) {
                sum += v;
            }
            observed_sum.store(sum, std::memory_order_relaxed);
            destroyed.store(true, std::memory_order_relaxed);
            delete r;
        }
    };

    std::vector<std::thread> owners;
    for (int i = 0; i < kNumOwners; ++i) {
        owners.emplace_back([res, i, &try_release_and_verify] {
            res->slots[i] = i + 1;
            try_release_and_verify(res);
        });
    }
    try_release_and_verify(res);  // this test's own reference races too
    for (auto& t : owners) {
        t.join();
    }

    constexpr int expected_sum = kNumOwners * (kNumOwners + 1) / 2;
    EXPECT_TRUE(destroyed.load());
    EXPECT_EQ(RefCountedResource::destroy_count.load(), 1);
    EXPECT_EQ(observed_sum.load(), expected_sum);
}
