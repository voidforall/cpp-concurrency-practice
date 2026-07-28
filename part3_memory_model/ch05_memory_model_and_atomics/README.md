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
2. Explain the difference between acquire, release, and sequentially consistent ordering in terms of what reorderings each forbids.
3. Give a real example where `memory_order_relaxed` is actually correct to use, and explain why.

## Definition of done
- [ ] Mini-project + katas run & test suite passes
- [ ] Self-quiz answered from memory

---
*Put declarations in `practice.hpp`, implementation in `practice.cpp`, tests in `test_practice.cpp`.
Build with CMake from the repo root (see top-level README).*
