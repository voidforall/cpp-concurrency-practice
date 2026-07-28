# Chapter 09: Advanced Thread Management

**Part V — Advanced Thread Mgmt & Parallel Algorithms** · Track: **FULL (new material — read, practice, quiz)**

## Key concepts
- Thread pools with work stealing
- Cooperative thread interruption (atomic cancel flag, or C++20 `jthread` + `stop_token`)
- Thread-local storage
- Propagating exceptions across thread boundaries (`std::exception_ptr`)

## Mini-project
Extend the Ch08 thread pool with either (a) work stealing (per-thread local queues, idle threads steal from others) or (b) cooperative cancellation via `jthread`/`stop_token` — pick one and implement it fully.

## Katas
1. Implement an `interruptible_thread` wrapper with an atomic cancel flag checked at cooperative checkpoints, plus a variant that can interrupt a blocking `condition_variable::wait`.
2. Make a pool worker's thrown exception observable by the submitter via `std::exception_ptr`/`std::promise`, and write a test that asserts the original exception type/message survives the round trip.

## Self-quiz (answer without notes)
1. Why can't you forcibly terminate a running `std::thread` from outside — what has to cooperate, and how?
2. How does work stealing improve load balance compared to a single shared task queue, and what does it cost?
3. Walk through exactly how an exception thrown inside a worker thread gets back to code running on a different thread.

## Definition of done
- [ ] Mini-project + katas run & test suite passes
- [ ] Self-quiz answered from memory

---
*Put declarations in `practice.hpp`, implementation in `practice.cpp`, tests in `test_practice.cpp`.
Build with CMake from the repo root (see top-level README).*
