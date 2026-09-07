#include <gtest/gtest.h>
#include "practice.hpp"

#include <atomic>
#include <string>
#include <thread>
#include <vector>

// Tests for Chapter 06: Designing Lock-Based Concurrent Data Structures.

TEST(ConcurrentHashMap, FindOnMissingKeyReturnsNullopt) {
    ConcurrentHashMap<int, std::string> map;

    EXPECT_EQ(map.find(42), std::nullopt);
}

TEST(ConcurrentHashMap, InsertThenFindReturnsValue) {
    ConcurrentHashMap<int, std::string> map;

    map.insert_or_assign(1, "one");

    ASSERT_TRUE(map.find(1).has_value());
    EXPECT_EQ(*map.find(1), "one");
}

TEST(ConcurrentHashMap, InsertOrAssignOverwritesExistingKey) {
    ConcurrentHashMap<int, std::string> map;

    map.insert_or_assign(1, "one");
    map.insert_or_assign(1, "uno");

    ASSERT_TRUE(map.find(1).has_value());
    EXPECT_EQ(*map.find(1), "uno");
}

TEST(ConcurrentHashMap, EraseRemovesKey) {
    ConcurrentHashMap<int, std::string> map;

    map.insert_or_assign(1, "one");
    map.erase(1);

    EXPECT_EQ(map.find(1), std::nullopt);
}

TEST(ConcurrentHashMap, EraseOnMissingKeyIsNoop) {
    ConcurrentHashMap<int, std::string> map;

    // Should not throw or crash even though the key was never inserted.
    map.erase(999);

    EXPECT_EQ(map.find(999), std::nullopt);
}

TEST(ConcurrentHashMap, KeysThatCollideIntoSameBucketAreBothRetrievable) {
    // Default bucket count is 19, so keys 0 and 19 hash to the same bucket.
    ConcurrentHashMap<int, int> map(19);

    map.insert_or_assign(0, 100);
    map.insert_or_assign(19, 119);

    ASSERT_TRUE(map.find(0).has_value());
    ASSERT_TRUE(map.find(19).has_value());
    EXPECT_EQ(*map.find(0), 100);
    EXPECT_EQ(*map.find(19), 119);

    map.erase(0);
    EXPECT_EQ(map.find(0), std::nullopt);
    ASSERT_TRUE(map.find(19).has_value());
    EXPECT_EQ(*map.find(19), 119);
}

TEST(ConcurrentHashMap, ConcurrentInsertsOfDistinctKeysAllSucceed) {
    ConcurrentHashMap<int, int> map;
    constexpr int num_threads = 8;
    constexpr int keys_per_thread = 1000;

    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&map, t] {
            for (int i = 0; i < keys_per_thread; ++i) {
                int key = t * keys_per_thread + i;
                map.insert_or_assign(key, key * 2);
            }
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }

    for (int t = 0; t < num_threads; ++t) {
        for (int i = 0; i < keys_per_thread; ++i) {
            int key = t * keys_per_thread + i;
            ASSERT_TRUE(map.find(key).has_value()) << "missing key " << key;
            EXPECT_EQ(*map.find(key), key * 2);
        }
    }
}

TEST(ConcurrentHashMap, ConcurrentReadersAndWritersOnSameKeyDoNotCrashOrTear) {
    // All threads hammer the same key, which forces contention on a single
    // bucket's mutex. This mainly guards against a Bucket::find or
    // Bucket::insert_or_assign that forgets to take its lock.
    ConcurrentHashMap<int, int> map;
    map.insert_or_assign(0, 0);

    std::atomic<bool> stop{false};
    std::atomic<int> writes{0};

    std::vector<std::thread> writers;
    for (int t = 0; t < 4; ++t) {
        writers.emplace_back([&map, &stop, &writes, t] {
            int value = t;
            while (!stop.load(std::memory_order_relaxed)) {
                map.insert_or_assign(0, value);
                value += 4;
                writes.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    std::vector<std::thread> readers;
    for (int t = 0; t < 4; ++t) {
        readers.emplace_back([&map, &stop] {
            while (!stop.load(std::memory_order_relaxed)) {
                // A torn read would surface as a value that never got written,
                // or (more likely) as a crash/UB under tsan.
                auto value = map.find(0);
                EXPECT_TRUE(value.has_value());
            }
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    stop.store(true, std::memory_order_relaxed);

    for (auto& thread : writers) {
        thread.join();
    }
    for (auto& thread : readers) {
        thread.join();
    }

    EXPECT_GT(writes.load(), 0);
}

TEST(ConcurrentHashMap, ConcurrentEraseAndInsertOnDisjointKeysConverge) {
    ConcurrentHashMap<int, int> map;
    constexpr int num_keys = 500;

    for (int i = 0; i < num_keys; ++i) {
        map.insert_or_assign(i, i);
    }

    std::thread eraser([&map] {
        for (int i = 0; i < num_keys; i += 2) {
            map.erase(i);
        }
    });
    std::thread inserter([&map] {
        for (int i = num_keys; i < num_keys * 2; ++i) {
            map.insert_or_assign(i, i);
        }
    });
    eraser.join();
    inserter.join();

    for (int i = 0; i < num_keys; i += 2) {
        EXPECT_EQ(map.find(i), std::nullopt) << "key " << i << " should have been erased";
    }
    for (int i = 1; i < num_keys; i += 2) {
        ASSERT_TRUE(map.find(i).has_value()) << "odd key " << i << " should still be present";
        EXPECT_EQ(*map.find(i), i);
    }
    for (int i = num_keys; i < num_keys * 2; ++i) {
        ASSERT_TRUE(map.find(i).has_value()) << "new key " << i << " should have been inserted";
        EXPECT_EQ(*map.find(i), i);
    }
}
