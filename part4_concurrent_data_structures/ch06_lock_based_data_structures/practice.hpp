// Chapter 06: Designing Lock-Based Concurrent Data Structures — practice.
//
// Mini-project: Implement a thread-safe hash-map-like lookup table using fine-grained per-bucket locking (an array of buckets, each guarded by its own mutex) supporting concurrent `get`/`insert`/`erase` from multiple threads.
//
// Declare your types/functions here; implement in practice.cpp.

#ifndef CH06_LOCK_BASED_DATA_STRUCTURES_PRACTICE_HPP
#define CH06_LOCK_BASED_DATA_STRUCTURES_PRACTICE_HPP

#include <algorithm>
#include <atomic>
#include <chrono>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <thread>
#include <vector>

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

// Kata 1: Hand over hand thread safe singly linked list
// Core principle: always acquire the locks according to relative ordering of the nodes
template <typename T>
class ConcurrentList {
private:
    struct Node {
      mutable std::mutex mutex;
      std::optional<T> data;
      std::unique_ptr<Node> next;

      Node() = default;
      explicit Node(T value) : data(std::move(value)) {}
    };
  
    Node head_;

public:
    void push_front(T value) {
      auto new_node = std::make_unique<Node>(std::move(value));

      std::lock_guard lock(head_.mutex);
      new_node->next = std::move(head_.next);
      head_.next = std::move(new_node);
    }

    template<typename Predicate>
    void remove_if(Predicate pred) {
      std::unique_lock<std::mutex> prev_lock(head_.mutex);
      Node* prev = &head_;

      while (Node* current = prev->next.get()) {
        std::unique_lock<std::mutex> current_lock(current->mutex);

        if (pred(*current->data)) {
          std::unique_ptr<Node> owned = std::move(prev->next);
          prev->next = std::move(current->next);
          current_lock.unlock();

          continue;
        }

        prev_lock.unlock();
        prev_lock = std::move(current_lock);
        prev = current;
      }
    }

    template<typename Function>
    void for_each(Function f) const {
      std::unique_lock<std::mutex> prev_lock(head_.mutex);
      const Node* current = head_.next.get();

      while (current) {
        std::unique_lock<std::mutex> current_lock(current->mutex);
        prev_lock.unlock();
        f(*current->data);
        prev_lock = std::move(current_lock);
        current = current->next.get();
      }
    }
};

// Kata 2: fine-grained-locking queue (separate head/tail mutexes) vs. a
// single-mutex baseline, benchmarked below under many-producer/many-consumer
// load.
//
// Push only ever takes tail_mutex_; try_pop only ever takes head_mutex_ (plus
// a brief tail_mutex_ acquisition inside get_tail() to check for emptiness).
// A dummy tail node means push and pop touch disjoint nodes whenever the
// queue holds more than one element, so producers and consumers can proceed
// in parallel with each other - they only ever contend within their own role.
template <typename T>
class FineGrainedQueue {
private:
    struct Node {
        std::shared_ptr<T> data;
        std::unique_ptr<Node> next;
    };

    std::unique_ptr<Node> head_;
    Node* tail_;
    mutable std::mutex head_mutex_;
    mutable std::mutex tail_mutex_;

    Node* get_tail() const {
        std::lock_guard lock(tail_mutex_);
        return tail_;
    }

    std::unique_ptr<Node> pop_head() {
        std::unique_ptr<Node> old_head = std::move(head_);
        head_ = std::move(old_head->next);
        return old_head;
    }

public:
    FineGrainedQueue() : head_(std::make_unique<Node>()), tail_(head_.get()) {}
    FineGrainedQueue(const FineGrainedQueue&) = delete;
    FineGrainedQueue& operator=(const FineGrainedQueue&) = delete;

    void push(T value) {
        auto data = std::make_shared<T>(std::move(value));
        auto new_tail = std::make_unique<Node>();
        Node* new_tail_ptr = new_tail.get();

        std::lock_guard lock(tail_mutex_);
        tail_->data = std::move(data);
        tail_->next = std::move(new_tail);
        tail_ = new_tail_ptr;
    }

    std::optional<T> try_pop() {
        std::lock_guard lock(head_mutex_);

        if (head_.get() == get_tail()) return std::nullopt;

        std::unique_ptr<Node> old_head = pop_head();
        return std::move(*old_head->data);
    }
};

// Single-mutex baseline queue for the kata 2 throughput comparison.
template <typename T>
class SingleMutexQueue {
private:
    mutable std::mutex mutex_;
    std::queue<T> data_;

public:
    void push(T value) {
        std::lock_guard lock(mutex_);
        data_.push(std::move(value));
    }

    std::optional<T> try_pop() {
        std::lock_guard lock(mutex_);

        if (data_.empty()) return std::nullopt;

        T value = std::move(data_.front());
        data_.pop();
        return value;
    }
};

// Runs num_producers threads each pushing items_per_producer items, and
// num_consumers threads try_pop-spinning until every pushed item has been
// consumed. Returns elapsed wall time in milliseconds.
template <typename Queue>
long long benchmark_queue(int num_producers, int num_consumers, long long items_per_producer) {
    Queue queue;
    const long long total_items = static_cast<long long>(num_producers) * items_per_producer;
    std::atomic<long long> consumed{0};

    auto produce = [&queue, items_per_producer] {
        for (long long i = 0; i < items_per_producer; ++i) {
            queue.push(static_cast<int>(i));
        }
    };

    auto consume = [&queue, &consumed, total_items] {
        while (consumed.load(std::memory_order_relaxed) < total_items) {
            if (queue.try_pop()) {
                consumed.fetch_add(1, std::memory_order_relaxed);
            }
        }
    };

    auto start = std::chrono::steady_clock::now();

    std::vector<std::thread> threads;
    threads.reserve(static_cast<std::size_t>(num_producers + num_consumers));
    for (int i = 0; i < num_producers; ++i) threads.emplace_back(produce);
    for (int i = 0; i < num_consumers; ++i) threads.emplace_back(consume);
    for (auto& t : threads) t.join();

    auto elapsed = std::chrono::steady_clock::now() - start;
    return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
}

#endif // CH06_LOCK_BASED_DATA_STRUCTURES_PRACTICE_HPP
