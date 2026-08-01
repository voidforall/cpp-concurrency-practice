// Chapter 01: Hello, World of Concurrency in C++! — practice.
//
// Mini-project: Build a `scoped_thread` RAII wrapper around `std::thread` that joins in its destructor, is non-copyable, and throws (or asserts) if constructed from a non-joinable thread.
//
// Declare your types/functions here; implement in practice.cpp.

#ifndef CH01_HELLO_WORLD_OF_CONCURRENCY_PRACTICE_HPP
#define CH01_HELLO_WORLD_OF_CONCURRENCY_PRACTICE_HPP

#include <vector>

// Kata 1:
// Launch N threads to calculate the partial sums of a vector of numbers.
// Each thread should calculate the sum of a portion of the vector and store the result in the corresponding index of the `partial_sums` vector.
void calculate_partial_sum(std::vector<int> const &numbers, int const num_threads, std::vector<int>& partial_sums);

#endif // CH01_HELLO_WORLD_OF_CONCURRENCY_PRACTICE_HPP
