// Chapter 06: Designing Lock-Based Concurrent Data Structures — practice.
//
// Mini-project: Implement a thread-safe hash-map-like lookup table using fine-grained per-bucket locking (an array of buckets, each guarded by its own mutex) supporting concurrent `get`/`insert`/`erase` from multiple threads.
//
// Declare your types/functions here; implement in practice.cpp.

#ifndef CH06_LOCK_BASED_DATA_STRUCTURES_PRACTICE_HPP
#define CH06_LOCK_BASED_DATA_STRUCTURES_PRACTICE_HPP

#include <list>
#include <vector>
#include <optional>
#include <shared_mutex>
#include <algorithm>

// Bucket level fine-grained thread safe hash map
template <typename Key, typename Value>
class ConcurrentHashMap {
private:
    class Bucket {
    public:
        std::optional<Value> find(const Key& key) const {
            std::shared_lock lock(mutex_);

            for (const auto& [k, v] : data_) {
                if (k == key) return v;
            }

            return std::nullopt;
        }

        void insert_or_assign(Key key, Value value) {
            std::unique_lock lock(mutex_);

            for (auto& [k, v] : data_) {
                if (k == key) {
                    v = std::move(value);
                    return;
                }
            }

            data_.emplace_back(std::move(key), std::move(value));
        }

        void erase(const Key& key) {
            std::unique_lock lock(mutex_);

            std::erase_if(data_, [&key](const auto& kv) { return kv.first == key; });
        }

    private:
        std::list<std::pair<Key, Value>> data_;
        mutable std::shared_mutex mutex_;
    };

    std::vector<Bucket> buckets_;

    Bucket& bucket_for(const Key& key) {
        std::size_t index = std::hash<Key>{}(key) % buckets_.size();

        return buckets_[index];
    }

    const Bucket& bucket_for(const Key& key) const {
        std::size_t index = std::hash<Key>{}(key) % buckets_.size();

        return buckets_[index];
    }

public:
    explicit ConcurrentHashMap(std::size_t bucket_count = 19) : buckets_(bucket_count) {}

    std::optional<Value> find(const Key& key) const {
        return bucket_for(key).find(key);
    }

    void insert_or_assign(Key key, Value value) {
        bucket_for(key).insert_or_assign(std::move(key), std::move(value));
    }

    void erase(const Key& key) {
        bucket_for(key).erase(key);
    }
};

#endif // CH06_LOCK_BASED_DATA_STRUCTURES_PRACTICE_HPP
