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

   **A:** A bare `wait(lock)` risks a **missed wakeup**: if `notify_one()`/`notify_all()` fires before the waiter ever calls `wait()`, the notification is simply lost - condition variables don't queue notifications, they only wake threads that are actually waiting at that moment - and the waiter then blocks forever, since nobody will notify again. Demonstrated directly in kata 1: a bare-wait version hung 6/6 runs. The predicate overload, `wait(lock, predicate)`, is equivalent to `while (!predicate()) wait(lock);` - checking the predicate *before* blocking closes the missed-wakeup window entirely (if the condition's already true, it never blocks at all), and the same loop also happens to protect against genuine spurious wakeups (the OS is allowed to wake a waiting thread with no `notify()` at all) as a side effect - one mechanism fixes both hazards.

   ```cpp
   cv.wait(lock);                              // missed-wakeup risk
   cv.wait(lock, [&ready] { return ready; });   // safe: checks first, loops on spurious/irrelevant wakeups
   ```

2. What's the practical difference between `std::launch::async` and `std::launch::deferred`?

   **A:** Not just *when* - **which thread**. `launch::async` guarantees the callable runs on a new thread, concurrently, starting right away. `launch::deferred` doesn't run it at all until `.get()`/`.wait()` is called - and when it finally does run, it executes **synchronously on the calling thread**, never concurrently. Code that "looks async" (`std::async(f, args...)` with no explicit policy) can silently become fully sequential if the implementation picks `deferred` - which is why `std::launch::async` was passed explicitly everywhere in this chapter's code (`run_producer_consumer_pipeline_async`, `count_primes_parallel_async`).

3. What does a `promise`/`future` pair give you that a raw condition_variable + shared bool doesn't?

   **A:** A typed **payload**, not just a signal. A raw condvar + bool only tells you "something happened" - any actual result has to be bolted on as a separate variable, and any failure path has to be hand-rolled. `promise`/`future` bundles the ready-signal *and* a result (or exception) together, delivered exactly once: if the promise side throws, `.get()` on the future side rethrows that same exception on the consuming thread - a raw condvar+bool gives you nothing for that.

## Definition of done
- [x] Kata(s) run & test suite passes
- [x] Self-quiz answered from memory

---
*Put declarations in `practice.hpp`, implementation in `practice.cpp`, tests in `test_practice.cpp`.
Build with CMake from the repo root (see top-level README).*
