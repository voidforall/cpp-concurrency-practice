# Chapter 10: Parallel Algorithms

**Part V — Advanced Thread Mgmt & Parallel Algorithms** · Track: **FULL (new material — read, practice, quiz)**

## Key concepts
- C++17 execution policies: `seq`, `par`, `par_unseq`, `unseq`
- Parallel standard algorithms: `for_each`, `sort`, `reduce`, `transform_reduce`
- When parallel algorithms help vs. hurt (overhead, data size, associativity requirements)

## Mini-project
Hand-roll a `parallel_accumulate` (chunked across `hardware_concurrency()` threads), then compare its correctness and performance against `std::reduce`/`std::transform_reduce` with `std::execution::par` on the same dataset.

## Katas
1. Parallelize a large-vector sort with `std::sort(std::execution::par, ...)` and benchmark it against sequential `std::sort` across multiple sizes to find the crossover point where parallelism starts winning.
2. Compute a dot product with `std::transform_reduce` under `execution::par_unseq`, and in the README explain why the reduction operation must be associative/commutative for the parallel version to be valid.

## Self-quiz (answer without notes)
1. What do `par`, `par_unseq`, and `unseq` each additionally permit the implementation to do, versus `seq`?
2. Why must the binary operation passed to a parallel reduce be associative (and often commutative)?
3. Describe a realistic case where reaching for a parallel algorithm would make code slower, not faster.

## Definition of done
- [ ] Mini-project + katas run & test suite passes
- [ ] Self-quiz answered from memory

---
*Put declarations in `practice.hpp`, implementation in `practice.cpp`, tests in `test_practice.cpp`.
Build with CMake from the repo root (see top-level README).*
