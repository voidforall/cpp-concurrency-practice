# Chapter 06: Designing Lock-Based Concurrent Data Structures

**Part IV — Concurrent Data Structures & Code Design** · Track: **FULL (new material — read, practice, quiz)**

## Key concepts
- Guidelines for thread-safe interface design (avoiding interface-level races)
- Fine-grained locking vs. a single coarse mutex
- Thread-safe queue with condition_variable + fine-grained locks
- Thread-safe lookup table with per-bucket locking
- Thread-safe singly linked list with hand-over-hand locking

## Mini-project
Implement a thread-safe hash-map-like lookup table using fine-grained per-bucket locking (an array of buckets, each guarded by its own mutex) supporting concurrent `get`/`insert`/`erase` from multiple threads.

## Katas
1. Implement a singly linked list supporting concurrent `push_front` and `remove_if` using hand-over-hand locking (lock node N+1 before releasing node N).
2. Benchmark your fine-grained-locking queue against a single-mutex queue under many-producer/many-consumer load; report the throughput crossover point.

## Self-quiz (answer without notes)
1. Why does a single coarse-grained mutex around an entire data structure become a scalability bottleneck as thread count grows?
2. What invariant must hand-over-hand locking preserve at every step to stay both correct and deadlock-free?
3. How many buckets/locks would you choose for a fine-grained lookup table, and what's the tradeoff in picking too many vs. too few?

## Definition of done
- [ ] Mini-project + katas run & test suite passes
- [ ] Self-quiz answered from memory

---
*Put declarations in `practice.hpp`, implementation in `practice.cpp`, tests in `test_practice.cpp`.
Build with CMake from the repo root (see top-level README).*
