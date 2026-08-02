// Chapter 02: Managing Threads — practice.
//
// Mini-project: Write a `parallel_for(first, last, f)` that partitions a range across `hardware_concurrency()` worker threads, each processing a contiguous chunk, then joins all of them.
//
// Declare your types/functions here; implement in practice.cpp.

#ifndef CH02_MANAGING_THREADS_PRACTICE_HPP
#define CH02_MANAGING_THREADS_PRACTICE_HPP

#include <algorithm>
#include <iterator>
#include <stop_token>
#include <thread>
#include <vector>
#include <concepts>

template<typename Iterator, typename Function>
    requires std::random_access_iterator<Iterator> &&
             std::invocable<Function, std::iter_reference_t<Iterator>>
void parallel_for(Iterator first, Iterator last, Function f) {
    size_t total_elements = std::distance(first, last);
    if (total_elements == 0) {
        return;
    }

    // hardware_concurrency() may return 0 if not computable; on some
    // platforms (this one included, arm64) integer division by zero does
    // not trap, so an unguarded 0 here would silently spawn zero threads
    // and skip the entire range instead of crashing or doing any work.
    size_t num_threads = std::max(1u, std::thread::hardware_concurrency());
    // Never spawn more threads than there are elements to process.
    num_threads = std::min(num_threads, total_elements);

    size_t chunk_size = total_elements / num_threads;

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    auto worker = [&](Iterator chunk_first, Iterator chunk_last) {
        for (auto it = chunk_first; it != chunk_last; ++it) {
            f(*it);
        }
    };

    for (size_t i = 0; i < num_threads; ++i) {
        Iterator chunk_first = first + i * chunk_size;
        Iterator chunk_last = (i == num_threads - 1) ? last : chunk_first + chunk_size;

        threads.emplace_back(worker, chunk_first, chunk_last);
    }

    for (auto& t : threads) {
        t.join();
    }
}

// Overload with jthread implementation, which allows for cooperative cancellation via stop_token.
template<typename Iterator, typename Function>
    requires std::random_access_iterator<Iterator> &&
             std::invocable<Function, std::iter_reference_t<Iterator>>
void parallel_for(Iterator first, Iterator last, Function f, std::stop_token stop_token) {
    size_t total_elements = std::distance(first, last);
    if (total_elements == 0) {
        return;
    }

    size_t num_threads = std::max(1u, std::thread::hardware_concurrency());
    num_threads = std::min(num_threads, total_elements);

    size_t chunk_size = total_elements / num_threads;

    std::vector<std::jthread> threads;
    threads.reserve(num_threads);

    auto worker = [&](Iterator chunk_first, Iterator chunk_last, std::stop_token st) {
        for (auto it = chunk_first; it != chunk_last; ++it) {
            if (st.stop_requested()) {
                return; // Exit early if stop is requested
            }
            f(*it);
        }
    };

    for (size_t i = 0; i < num_threads; ++i) {
        Iterator chunk_first = first + i * chunk_size;
        Iterator chunk_last = (i == num_threads - 1) ? last : chunk_first + chunk_size;

        threads.emplace_back(worker, chunk_first, chunk_last, stop_token);
    }
}

#endif // CH02_MANAGING_THREADS_PRACTICE_HPP
