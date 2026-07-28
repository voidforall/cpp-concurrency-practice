# Chapter 02: Managing Threads

**Part I — Foundations** · Track: **REFRESHER (already read — confirm mastery)**

## Key concepts
- Passing arguments to threads (always copied/moved, not implicitly referenced)
- Dangling references when a detached thread outlives a local variable it captured by reference
- Transferring ownership of a thread (`std::move`, threads are move-only)
- `std::thread::hardware_concurrency()`
- Identifying threads via `get_id()`
- C++20 `std::jthread` and cooperative cancellation via `stop_token`

## Mini-project
Write a `parallel_for(first, last, f)` that partitions a range across `hardware_concurrency()` worker threads, each processing a contiguous chunk, then joins all of them.

## Katas
1. Deliberately capture a local variable by reference in a detached thread's lambda so it dangles after the function returns (observe the bug, e.g. with ASan); fix it by capturing by value.
2. Rewrite `parallel_for` using `std::jthread` and a `std::stop_token` so the caller can cooperatively cancel remaining work early.

## Self-quiz (answer without notes)
1. Why does `std::thread`'s constructor copy/move arguments into internal storage rather than binding by reference, even when the target function takes a reference parameter?
2. What UB can result from `detach()`-ing a thread that captured a stack-local variable by reference?
3. How does `std::jthread` differ from `std::thread` beyond auto-join in its destructor?

## Definition of done
- [ ] Kata(s) run & test suite passes
- [ ] Self-quiz answered from memory

---
*Put declarations in `practice.hpp`, implementation in `practice.cpp`, tests in `test_practice.cpp`.
Build with CMake from the repo root (see top-level README).*
