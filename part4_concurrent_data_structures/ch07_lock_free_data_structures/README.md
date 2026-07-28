# Chapter 07: Designing Lock-Free Concurrent Data Structures

**Part IV — Concurrent Data Structures & Code Design** · Track: **FULL (new material — read, practice, quiz)**

## Key concepts
- Lock-free vs. wait-free vs. obstruction-free
- `compare_exchange_weak` / `compare_exchange_strong` and CAS retry loops
- The ABA problem and mitigations (tagged pointers, hazard pointers, epoch-based reclamation)
- Memory reclamation as the hard part of lock-free design

## Mini-project
Implement a lock-free stack (Treiber stack) using `compare_exchange_weak` on an atomic head pointer, with a simple reference-counting scheme for safe memory reclamation.

## Katas
1. Build a naive CAS-based stack `pop()` that is vulnerable to the ABA problem; construct a scenario (simulated thread delays / interleavings) that demonstrates the bug, then fix it with tagged pointers or a hazard-pointer-lite scheme.
2. Implement lock-free `push()` for a queue and explain, in the README, exactly why the CAS loop must retry rather than fail on contention.

## Self-quiz (answer without notes)
1. What precisely distinguishes a lock-free algorithm from one that merely uses atomics internally (e.g. progress guarantees under thread preemption)?
2. Describe the ABA problem with a concrete example, and name two different fixes.
3. Why is memory reclamation specifically the hard part of lock-free data structures, more than the CAS logic itself?

## Definition of done
- [ ] Mini-project + katas run & test suite passes
- [ ] Self-quiz answered from memory

---
*Put declarations in `practice.hpp`, implementation in `practice.cpp`, tests in `test_practice.cpp`.
Build with CMake from the repo root (see top-level README).*
