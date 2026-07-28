# Chapter 03: Sharing Data Between Threads

**Part II — Sharing & Synchronizing Data** · Track: **REFRESHER (already read — confirm mastery)**

## Key concepts
- Race conditions and why they're a design problem, not just a locking problem
- `std::mutex`, `std::lock_guard`, `std::scoped_lock`
- Deadlock and lock ordering; `std::lock`/`std::scoped_lock` for locking multiple mutexes atomically
- Lazy initialization and `std::call_once`
- Interface-level races: why a thread-safe `empty()` + `pop()` can still race even if each is individually locked

## Mini-project
Implement a thread-safe stack whose `pop()` returns a `std::shared_ptr<T>` (or `std::optional<T>`) in one atomic-from-the-caller's-perspective call, so there's no separate `empty()`/`top()`/`pop()` race window.

## Katas
1. Deliberately deadlock two threads that lock two mutexes in opposite order; fix it with `std::scoped_lock`.
2. Implement a lazy-initialized singleton with `std::call_once`, then compare it against a naive (buggy) double-checked-locking version and explain why the naive version is unsafe pre-C++11 semantics.

## Self-quiz (answer without notes)
1. Why is it insufficient to just wrap each individual operation on a shared stack in its own mutex lock?
2. How does `std::lock` (or `std::scoped_lock` with multiple arguments) prevent deadlock when acquiring two mutexes?
3. What exactly does `std::call_once` guarantee about concurrent callers?

## Definition of done
- [ ] Kata(s) run & test suite passes
- [ ] Self-quiz answered from memory

---
*Put declarations in `practice.hpp`, implementation in `practice.cpp`, tests in `test_practice.cpp`.
Build with CMake from the repo root (see top-level README).*
