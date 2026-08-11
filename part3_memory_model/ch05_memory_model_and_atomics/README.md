# Chapter 05: The C++ Memory Model and Operations on Atomic Types

**Part III — Memory Model & Atomics** · Track: **FULL (new material — read, practice, quiz)**

## Key concepts
- Modification order and happens-before relationships
- `std::atomic<T>` and the operations it supports
- Memory orderings: `relaxed`, `acquire`, `release`, `acq_rel`, `seq_cst`
- `std::atomic_flag` and building a spinlock
- Fences (`std::atomic_thread_fence`)

## Mini-project
Implement a spinlock using `std::atomic_flag` with correct acquire (on lock) / release (on unlock) ordering. Benchmark it against `std::mutex` under light and heavy contention.

## Katas
1. Implement a single-producer/single-consumer handoff using a `std::atomic<bool>` ready-flag with acquire/release ordering; then deliberately weaken it to `memory_order_relaxed` and explain (with reasoning, not just code) what guarantee breaks.
2. Implement a simple intrusive reference counter (increment relaxed, decrement release, and an acquire fence/read only on the count-reaches-zero path) mirroring how `shared_ptr`'s control block stays correct without a full mutex.

## Self-quiz (answer without notes)
1. What does "happens-before" mean, and why do plain (non-atomic) reads/writes racing without it constitute undefined behavior?

   **A:** "Happens-before" is a formal ordering relationship between actions in a program, built from two pieces plus transitivity: **sequenced-before** (program order within a single thread) and **synchronizes-with** (e.g. one thread's acquire-load reading the value written by another thread's release-store on the *same* atomic). If A happens-before B and B happens-before C, then A happens-before C, chaining ordering across threads.

   The standard defines a **data race** as two-or-more threads accessing the same memory location, at least one a write, with no happens-before relationship between them - and a program containing a data race has undefined behavior. The *causal* reason, not just "the standard says so": without happens-before, the compiler and CPU are both free to reorder, cache in registers, eliminate, or hoist non-atomic reads/writes, on the assumption that no other thread is watching that location without synchronization. Making data races UB (rather than promising something milder like "you'll just see a stale value") is exactly what gives implementations that optimization freedom - guaranteeing anything weaker would force compilers to preserve far more ordering than they currently do, a real performance cost across *all* code, not just racy code. Demonstrated directly in kata 1: `SpscHandoffRelaxed` is a genuine data race on `data`, confirmed with ThreadSanitizer, even though it showed 0/100000 failures by value on this hardware - UB doesn't have to visibly misbehave to be real.

2. Explain the difference between acquire, release, and sequentially consistent ordering in terms of what reorderings each forbids.

   **A:** Acquire and release are not really "a pair" - each has its own individual constraint:
   - **acquire** (on a load): forbids moving any *later* read/write (in program order) to *before* it.
   - **release** (on a store): forbids moving any *earlier* read/write (in program order) to *after* it.

   Together, when one thread's acquire-load reads the value from another thread's release-store on the *same* atomic, that pair establishes happens-before between them - everything before the release becomes visible to everything after the acquire.

   **seq_cst** isn't just "stronger acquire/release" - its distinguishing feature is a single **total order** that *every thread agrees on*, across *all* seq_cst operations on *all* variables. Acquire/release alone doesn't give you that: two unrelated acquire/release pairs (e.g. on two different atomics, touched by different thread pairs) can disagree about which happened first, since acquire/release only orders things *within* a specific synchronizes-with chain, not globally. Classic illustration: two threads each store to their own flag, then load the other's flag - with seq_cst, at least one thread is guaranteed to observe the other's store; with mere acquire/release on the two independent flags, both threads could observe "I stored first," since nothing ties the two unrelated variables into a shared order.

3. Give a real example where `memory_order_relaxed` is actually correct to use, and explain why.

   **A:** `IntrusiveRefCounted::add_ref()` (kata 2):

   ```cpp
   void add_ref() {
       count_.fetch_add(1, std::memory_order_relaxed);
   }
   ```

   Relaxed is correct here because the calling thread already holds a valid reference - that's the only way it could be calling `add_ref()` at all - so there's no *new* data being published to other threads by this call. The only property that actually matters is that the increment itself is atomic (no lost updates under concurrent `fetch_add`s); there's nothing else that needs a happens-before edge attached to it. Contrast with `release()`'s decrement, which *does* need release ordering, since it's guarding writes made while the reference was held, that the eventual destroying thread must be able to see.

## Definition of done
- [x] Mini-project + katas run & test suite passes
- [x] Self-quiz answered from memory

---
*Put declarations in `practice.hpp`, implementation in `practice.cpp`, tests in `test_practice.cpp`.
Build with CMake from the repo root (see top-level README).*
