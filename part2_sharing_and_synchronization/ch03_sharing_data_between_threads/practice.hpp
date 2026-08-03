// Chapter 03: Sharing Data Between Threads — practice.
//
// Mini-project: Implement a thread-safe stack whose `pop()` returns a `std::shared_ptr<T>` (or `std::optional<T>`) in one atomic-from-the-caller's-perspective call, so there's no separate `empty()`/`top()`/`pop()` race window.
//
// Declare your types/functions here; implement in practice.cpp.

#ifndef CH03_SHARING_DATA_BETWEEN_THREADS_PRACTICE_HPP
#define CH03_SHARING_DATA_BETWEEN_THREADS_PRACTICE_HPP

#include <stack>
#include <mutex>
#include <memory>
#include <thread>
#include <chrono>

// Kata 2: lazy-initialized singleton via std::call_once, compared against a
// naive (buggy) double-checked-locking version.

class LazySingleton {
public:
    LazySingleton() {
        // Widen the construction window so a concurrent unsynchronized read
        // (in the naive version below) has a realistic chance to race
        // against it, instead of the whole thing finishing before any other
        // thread gets scheduled.
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        value_ = 42;
    }

    int value() const { return value_; }

private:
    int value_;
};

// Correct: std::call_once guarantees the lambda runs exactly once across all
// callers, and that every caller - whether it ran the lambda, was blocked
// waiting for it, or arrives later - observes the fully-constructed result.
inline LazySingleton& get_instance_call_once() {
    static std::once_flag flag;
    static LazySingleton* instance = nullptr;
    std::call_once(flag, [] { instance = new LazySingleton(); });
    return *instance;
}

// Naive (buggy) double-checked locking: the first `if` reads `instance`
// with no synchronization at all. Even though only the locked branch ever
// writes to it, an unsynchronized read racing against a write is a data
// race - and `instance = new LazySingleton()` is not one atomic step
// (allocate, construct, then assign the pointer), so without a proven
// happens-before edge, another thread's unsynchronized read could observe
// a non-null `instance` before construction has actually finished.
inline LazySingleton* get_instance_naive() {
    static std::mutex mtx;
    static LazySingleton* instance = nullptr;

    if (instance == nullptr) {  // unsynchronized read - the bug
        std::lock_guard<std::mutex> lock(mtx);
        if (instance == nullptr) {
            instance = new LazySingleton();
        }
    }
    return instance;
}

template <typename T>
class ThreadSafeStack {
public:
    ThreadSafeStack() = default;

    // std::mutex is neither copyable nor movable, which would otherwise make
    // these implicitly deleted anyway - declared explicitly so the choice is
    // documented rather than an accidental side effect of the mutex member.
    ThreadSafeStack(const ThreadSafeStack&) = delete;
    ThreadSafeStack& operator=(const ThreadSafeStack&) = delete;

    void push(T value) {
        std::lock_guard<std::mutex> lock(mutex_);
        stack_.push(std::move(value));
    }

    // top() and pop() are combined into a single atomic operation to avoid race
    std::shared_ptr<T> pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stack_.empty()) {
            return nullptr;
        }
        auto value = std::make_shared<T>(std::move(stack_.top()));
        stack_.pop();
        return value;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return stack_.empty();
    }

private:
    std::stack<T> stack_;
    mutable std::mutex mutex_;
};

#endif // CH03_SHARING_DATA_BETWEEN_THREADS_PRACTICE_HPP
