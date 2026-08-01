#include "practice.hpp"

#include <thread>
#include <print>

// Chapter 01: Hello, World of Concurrency in C++! — practice.
//

// Kata 1: Launch N threads to calculate the partial sums of a vector of numbers.
void calculate_partial_sum(std::vector<int> const &numbers, int const num_threads, std::vector<int>& partial_sums) {
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    partial_sums.resize(num_threads, 0);

    auto worker = [&](int thread_id) {
        int chunk_size = numbers.size() / num_threads;
        int start_index = thread_id * chunk_size;
        int end_index = (thread_id == num_threads - 1) ? numbers.size() : start_index + chunk_size;

        std::println("Thread {} calculating sum from index {} to {}", thread_id, start_index, end_index - 1);

        for (int i = start_index; i < end_index; ++i) {
            partial_sums[thread_id] += numbers[i];
        }

        std::println("Thread {} finished with partial sum: {}", thread_id, partial_sums[thread_id]);
    };

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker, i);
    }

    for (auto& t : threads) {
        t.join();
    }
}