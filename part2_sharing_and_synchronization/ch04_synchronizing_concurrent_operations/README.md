# Chapter 04: Synchronizing Concurrent Operations

**Part II — Sharing & Synchronizing Data** · Track: **REFRESHER (already read — confirm mastery)**

## Key concepts
- `std::condition_variable`: `wait`, `notify_one`, `notify_all`, spurious wakeups and predicate-based waiting
- `std::future`, `std::async`, `std::promise`, `std::packaged_task`
- Waiting with timeouts: `wait_for`/`wait_until`
- C++20 `std::latch` and `std::barrier`

## Mini-project
Build a bounded producer-consumer queue using a mutex + condition_variable (predicate-based wait for both 'not full' and 'not empty'), then re-implement the same producer/consumer pipeline using `std::async` and compare the two designs.

## Katas
1. Write a condition_variable `wait()` call *without* a predicate, engineer a missed-wakeup/spurious-wakeup bug, then fix it by waiting on a predicate.
2. Use `std::async` to fan out several independent computations and gather results with `future::get()`; measure it against a purely sequential version.

## Self-quiz (answer without notes)
1. Why must `condition_variable::wait()` almost always be called with a predicate — what specific bug does omitting it cause (not just "spurious wakeups")?
2. What's the practical difference between `std::launch::async` and `std::launch::deferred`?
3. What does a `promise`/`future` pair give you that a raw condition_variable + shared bool doesn't?

## Definition of done
- [ ] Kata(s) run & test suite passes
- [ ] Self-quiz answered from memory

---
*Put declarations in `practice.hpp`, implementation in `practice.cpp`, tests in `test_practice.cpp`.
Build with CMake from the repo root (see top-level README).*
