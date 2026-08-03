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

   **A:** Because a *sequence* of two individually-locked calls still isn't atomic as a whole. Even if `top()` and `pop()` are each separately mutex-protected, a thread can read via `top()`, get preempted, another thread's `pop()` removes that same element, and now the first thread's own `pop()` either removes a *different* element than the one it just read, or finds the stack already empty:

   ```cpp
   // Racy even though each call is individually locked:
   if (!s.empty()) {        // (1) locked check
       T value = s.top();   // (2) locked read - another thread's pop() could run between (1)/(2)/(3)
       s.pop();              // (3) locked remove - might not be the same element read in (2)
   }
   ```

   The fix is combining "check + read + remove" into a single locked call — `ThreadSafeStack::pop()` returns the value and removes it atomically-from-the-caller's-perspective, closing the window entirely.

2. How does `std::lock` (or `std::scoped_lock` with multiple arguments) prevent deadlock when acquiring two mutexes?

   **A:** Not via one indivisible hardware operation - via a **try-lock-and-backoff algorithm**: it attempts to lock the mutexes one at a time using `try_lock`, and if any attempt fails (already held elsewhere), it releases everything it's acquired so far and retries. The observable guarantee is all-or-nothing: no thread is ever left holding a strict subset of the requested locks while blocked waiting on the rest indefinitely - which is exactly what breaks the circular-wait condition that causes deadlock, regardless of what order different threads pass the same mutexes in:

   ```cpp
   std::mutex a, b;

   // thread 1
   std::scoped_lock lock(a, b);
   // thread 2 - opposite order, still deadlock-free:
   std::scoped_lock lock(b, a);
   ```

3. What exactly does `std::call_once` guarantee about concurrent callers?

   **A:** The callable runs exactly once across every thread that calls `call_once` with the same `once_flag` - no matter how many threads race to trigger it - and every caller (whether it ran the callable, was blocked waiting for it, or arrives later) is guaranteed to observe the fully-completed result: a real happens-before relationship, not just "probably done by now."

   ```cpp
   std::once_flag flag;
   std::call_once(flag, [] { /* runs exactly once, ever */ });
   ```

   Bonus edge case: if the callable throws, the flag is *not* marked complete - a later call will retry the callable rather than the exception permanently "poisoning" that initialization.

## Definition of done
- [x] Kata(s) run & test suite passes
- [x] Self-quiz answered from memory

---
*Put declarations in `practice.hpp`, implementation in `practice.cpp`, tests in `test_practice.cpp`.
Build with CMake from the repo root (see top-level README).*
