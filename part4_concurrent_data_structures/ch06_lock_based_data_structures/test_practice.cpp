#include <gtest/gtest.h>
#include "practice.hpp"

#include <algorithm>
#include <atomic>
#include <mutex>
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

namespace {

template <typename T>
std::vector<T> collect(const ConcurrentList<T>& list) {
    std::vector<T> values;
    list.for_each([&values](const T& value) { values.push_back(value); });
    return values;
}

} // namespace

TEST(ConcurrentList, ForEachOnEmptyListVisitsNothing) {
    ConcurrentList<int> list;

    EXPECT_TRUE(collect(list).empty());
}

TEST(ConcurrentList, PushFrontAddsElement) {
    ConcurrentList<int> list;

    list.push_front(1);

    EXPECT_EQ(collect(list), std::vector<int>{1});
}

TEST(ConcurrentList, PushFrontMultipleTimesPreservesLifoOrder) {
    ConcurrentList<int> list;

    list.push_front(1);
    list.push_front(2);
    list.push_front(3);

    // Each push_front lands at the head, so the most recent push is first.
    EXPECT_EQ(collect(list), (std::vector<int>{3, 2, 1}));
}

TEST(ConcurrentList, RemoveIfOnEmptyListIsNoop) {
    ConcurrentList<int> list;

    list.remove_if([](int) { return true; });

    EXPECT_TRUE(collect(list).empty());
}

TEST(ConcurrentList, RemoveIfWithNoMatchesLeavesListUnchanged) {
    ConcurrentList<int> list;
    list.push_front(1);
    list.push_front(2);
    list.push_front(3);

    list.remove_if([](int value) { return value > 100; });

    EXPECT_EQ(collect(list), (std::vector<int>{3, 2, 1}));
}

TEST(ConcurrentList, RemoveIfRemovesMatchingHeadElement) {
    ConcurrentList<int> list;
    list.push_front(1);
    list.push_front(2);
    list.push_front(3);

    list.remove_if([](int value) { return value == 3; });

    EXPECT_EQ(collect(list), (std::vector<int>{2, 1}));
}

TEST(ConcurrentList, RemoveIfRemovesMatchingTailElement) {
    ConcurrentList<int> list;
    list.push_front(1);
    list.push_front(2);
    list.push_front(3);

    list.remove_if([](int value) { return value == 1; });

    EXPECT_EQ(collect(list), (std::vector<int>{3, 2}));
}

TEST(ConcurrentList, RemoveIfRemovesMatchingMiddleElement) {
    ConcurrentList<int> list;
    list.push_front(1);
    list.push_front(2);
    list.push_front(3);

    list.remove_if([](int value) { return value == 2; });

    EXPECT_EQ(collect(list), (std::vector<int>{3, 1}));
}

TEST(ConcurrentList, RemoveIfRemovesAllConsecutiveMatches) {
    // Regresses a hand-over-hand implementation that advances `prev` even
    // when the current node was just removed, which would skip the node
    // immediately after a match.
    ConcurrentList<int> list;
    for (int i = 5; i >= 1; --i) list.push_front(i);

    list.remove_if([](int value) { return value % 2 == 0; });

    EXPECT_EQ(collect(list), (std::vector<int>{1, 3, 5}));
}

TEST(ConcurrentList, RemoveIfMatchingEveryElementEmptiesList) {
    ConcurrentList<int> list;
    list.push_front(1);
    list.push_front(2);
    list.push_front(3);

    list.remove_if([](int) { return true; });

    EXPECT_TRUE(collect(list).empty());
}

TEST(ConcurrentList, ConcurrentPushFrontFromMultipleThreadsAllSucceed) {
    ConcurrentList<int> list;
    constexpr int num_threads = 8;
    constexpr int pushes_per_thread = 1000;

    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&list, t] {
            for (int i = 0; i < pushes_per_thread; ++i) {
                list.push_front(t * pushes_per_thread + i);
            }
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }

    std::vector<int> values = collect(list);
    ASSERT_EQ(values.size(), static_cast<std::size_t>(num_threads * pushes_per_thread));

    std::sort(values.begin(), values.end());
    for (int i = 0; i < num_threads * pushes_per_thread; ++i) {
        EXPECT_EQ(values[i], i);
    }
}

TEST(ConcurrentList, ConcurrentPushFrontAndRemoveIfDoNotCrashOrCorrupt) {
    // Hammers push_front and remove_if from separate threads at the same
    // time. This mainly guards against a hand-over-hand implementation that
    // ever holds zero locks, or two non-adjacent locks, exposing a window
    // where a node can be freed while another thread still points at it.
    ConcurrentList<int> list;

    std::atomic<bool> stop{false};
    std::atomic<int> pushes{0};

    std::vector<std::thread> pushers;
    for (int t = 0; t < 4; ++t) {
        pushers.emplace_back([&list, &stop, &pushes, t] {
            int value = t;
            while (!stop.load(std::memory_order_relaxed)) {
                list.push_front(value);
                value += 4;
                pushes.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    std::vector<std::thread> removers;
    for (int t = 0; t < 4; ++t) {
        removers.emplace_back([&list, &stop, t] {
            while (!stop.load(std::memory_order_relaxed)) {
                list.remove_if([t](int value) { return value % 4 == t; });
            }
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    stop.store(true, std::memory_order_relaxed);

    for (auto& thread : pushers) {
        thread.join();
    }
    for (auto& thread : removers) {
        thread.join();
    }

    // Drain whatever is left; a crash or hang here would indicate a race.
    list.remove_if([](int) { return true; });
    EXPECT_TRUE(collect(list).empty());
    EXPECT_GT(pushes.load(), 0);
}

// Both kata 2 queues share the same push/try_pop interface, so run the same
// correctness checks against each rather than duplicating the test bodies.
template <typename Queue>
class ConcurrentQueueTest : public ::testing::Test {};

using QueueTypes = ::testing::Types<FineGrainedQueue<int>, SingleMutexQueue<int>>;
TYPED_TEST_SUITE(ConcurrentQueueTest, QueueTypes);

TYPED_TEST(ConcurrentQueueTest, TryPopOnEmptyQueueReturnsNullopt) {
    TypeParam queue;

    EXPECT_EQ(queue.try_pop(), std::nullopt);
}

TYPED_TEST(ConcurrentQueueTest, PushThenTryPopReturnsValue) {
    TypeParam queue;
    queue.push(42);

    EXPECT_EQ(queue.try_pop(), 42);
}

TYPED_TEST(ConcurrentQueueTest, PreservesFifoOrder) {
    TypeParam queue;
    queue.push(1);
    queue.push(2);
    queue.push(3);

    EXPECT_EQ(queue.try_pop(), 1);
    EXPECT_EQ(queue.try_pop(), 2);
    EXPECT_EQ(queue.try_pop(), 3);
    EXPECT_EQ(queue.try_pop(), std::nullopt);
}

TYPED_TEST(ConcurrentQueueTest, ConcurrentProducersAndConsumersAccountForEveryItemExactlyOnce) {
    TypeParam queue;
    constexpr int num_producers = 8;
    constexpr int num_consumers = 4;
    constexpr int items_per_producer = 2000;
    constexpr int total_items = num_producers * items_per_producer;

    std::vector<std::thread> producers;
    for (int t = 0; t < num_producers; ++t) {
        producers.emplace_back([&queue, t] {
            for (int i = 0; i < items_per_producer; ++i) {
                queue.push(t * items_per_producer + i);
            }
        });
    }

    std::mutex consumed_mutex;
    std::vector<int> consumed;
    std::atomic<int> consumed_count{0};

    std::vector<std::thread> consumers;
    for (int t = 0; t < num_consumers; ++t) {
        consumers.emplace_back([&] {
            while (consumed_count.load(std::memory_order_relaxed) < total_items) {
                if (auto value = queue.try_pop()) {
                    std::lock_guard lock(consumed_mutex);
                    consumed.push_back(*value);
                    consumed_count.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }

    for (auto& thread : producers) thread.join();
    for (auto& thread : consumers) thread.join();

    ASSERT_EQ(consumed.size(), static_cast<std::size_t>(total_items));

    std::sort(consumed.begin(), consumed.end());
    for (int i = 0; i < total_items; ++i) {
        EXPECT_EQ(consumed[i], i) << "item " << i << " lost or duplicated";
    }
}
