# Chapter 08: Designing Concurrent Code

**Part IV — Concurrent Data Structures & Code Design** · Track: **FULL (new material — read, practice, quiz)**

## Key concepts
- Task-based vs. data-based work decomposition
- Contention, false sharing, and cache ping-pong
- Amdahl's law and scalability limits
- Exception safety across thread boundaries
- std::thread vs. thread pools

## Mini-project
Implement a minimal fixed-size thread pool (task queue, worker threads that pull and execute, graceful shutdown) and use it to parallelize a chunked numeric workload (e.g. parallel sum or row-chunked matrix multiply).

## Katas
1. Demonstrate false sharing: have threads write to adjacent elements of a small array vs. cache-line-padded elements, and measure the timing difference.
2. Measure your thread pool's speedup on the numeric workload at 1/2/4/8 threads and compare it against the theoretical prediction from Amdahl's law given your workload's serial fraction.

## Self-quiz (answer without notes)
1. What is false sharing, concretely, and what causes it at the cache-coherence level?
2. State Amdahl's law and explain what it implies about the point of diminishing returns for adding threads.
3. When would you choose task-based decomposition over data-based decomposition, or vice versa?

## Definition of done
- [ ] Mini-project + katas run & test suite passes
- [ ] Self-quiz answered from memory

---
*Put declarations in `practice.hpp`, implementation in `practice.cpp`, tests in `test_practice.cpp`.
Build with CMake from the repo root (see top-level README).*
