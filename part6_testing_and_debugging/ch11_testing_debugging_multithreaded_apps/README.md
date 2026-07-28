# Chapter 11: Testing and Debugging Multithreaded Applications

**Part VI — Testing & Debugging** · Track: **FULL (new material — read, practice, quiz)**

## Key concepts
- Categories of concurrency bugs: races, deadlocks, livelocks, lost wakeups
- Techniques for locating them: code review checklists, targeted stress tests, ThreadSanitizer
- Designing concurrent code for testability (injecting/mocking concurrency primitives)
- Structured logging strategies for diagnosing concurrency bugs

## Mini-project
Take a deliberately-reintroduced race from an earlier chapter's kata, run it under ThreadSanitizer (`-fsanitize=thread`) to catch the race, then fix it and confirm a clean TSan run.

## Katas
1. Write a stress-test harness that runs a data structure under N threads for M iterations with randomized delays, to surface races that a single normal run won't show.
2. Build a minimal lock-order logger: wrap mutex lock/unlock to log acquisition order across threads, then use the log to manually spot a lock-order inversion you inject on purpose.

## Self-quiz (answer without notes)
1. Why do race conditions frequently fail to reproduce under normal (non-stress, non-sanitized) testing?
2. What does ThreadSanitizer actually detect, and what classes of concurrency bugs does it *not* catch?
3. What makes concurrent code inherently hard to unit test, and what design choices make it easier?

## Definition of done
- [ ] Mini-project + katas run & test suite passes
- [ ] Self-quiz answered from memory

---
*Put declarations in `practice.hpp`, implementation in `practice.cpp`, tests in `test_practice.cpp`.
Build with CMake from the repo root (see top-level README).*
