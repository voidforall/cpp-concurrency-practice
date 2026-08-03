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
