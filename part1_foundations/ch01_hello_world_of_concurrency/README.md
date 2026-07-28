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
2. What's the practical difference between `join()` and `detach()`, and when is `detach()` actually safe to use?
3. Give a concrete example distinguishing concurrency from parallelism.

## Definition of done
- [ ] Kata(s) run & test suite passes
- [ ] Self-quiz answered from memory

---
*Put declarations in `practice.hpp`, implementation in `practice.cpp`, tests in `test_practice.cpp`.
Build with CMake from the repo root (see top-level README).*
