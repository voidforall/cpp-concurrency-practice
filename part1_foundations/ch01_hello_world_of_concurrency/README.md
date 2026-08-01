# Chapter 01: Hello, World of Concurrency in C++!

**Part I — Foundations** · Track: **REFRESHER (already read — confirm mastery)**

## Key concepts
- Concurrency vs. parallelism
- Why use concurrency (separation of concerns, performance)
- std::thread: launch, join, detach
- RAII thread guard (join-on-destruction)
- What happens if a joinable thread's destructor runs (std::terminate)

## Mini-project
Build a `scoped_thread` RAII wrapper around `std::thread` that joins in its destructor, is non-copyable, and throws (or asserts) if constructed from a non-joinable thread.

## Katas
1. Launch N threads that each compute a partial sum and print their `std::thread::id`; join all before exiting.
2. Write a program that crashes with `std::terminate` because a `std::thread` goes out of scope while still joinable; fix it using your `scoped_thread`.

## Self-quiz (answer without notes)
1. Why does a joinable `std::thread`'s destructor call `std::terminate` instead of implicitly joining or detaching?

   **A:** Both implicit options are unsafe in their own way, so the library refuses to silently pick one. An implicit *join* would make the destructor block — possibly indefinitely — and destructors often run during stack unwinding (exception propagation), where a silent hang is a far worse failure mode than a loud, immediate crash. An implicit *detach* would let the thread keep running after its enclosing scope (and anything it captured by reference from that scope) is destroyed — a dangling-reference bug. Calling `std::terminate()` forces the programmer to make an explicit, conscious choice (join or detach) rather than the library defaulting to a behavior that's unsafe somewhere else.

2. What's the practical difference between `join()` and `detach()`, and when is `detach()` actually safe to use?

   **A:** `join()` blocks the *calling* thread until the target thread's function returns, then reclaims the thread's resources and marks it non-joinable — it doesn't end the target's execution, the target ends on its own. `detach()` severs the thread object from the underlying thread of execution, letting it run independently in the background with no further way to wait on or signal it. `detach()` is safe when the thread is effectively daemon-like: it must not hold references to anything in the caller's scope that could be destroyed before the thread finishes (e.g. stack locals), and the caller genuinely doesn't need to know when it completes.

3. Give a concrete example distinguishing concurrency from parallelism.

   **A:** A producer/consumer pipeline running on separate threads is concurrency — the tasks make logically independent progress and don't need to execute at the literal same instant (they could be time-sliced on a single core). Splitting a scientific computation across threads pinned to different CPU cores is parallelism — the point is genuinely simultaneous execution to finish faster. Concurrency is about *structuring* independently-progressing tasks; parallelism is about *literally* doing multiple things at once.

## Definition of done
- [x] Kata(s) run & test suite passes
- [x] Self-quiz answered from memory

---
*Put declarations in `practice.hpp`, implementation in `practice.cpp`, tests in `test_practice.cpp`.
Build with CMake from the repo root (see top-level README).*
