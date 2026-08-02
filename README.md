# C++ Concurrency in Action — Practice

Hands-on practice, mini-projects, and self-quizzes while working through
*C++ Concurrency in Action, 2nd ed.* (Anthony Williams). Tracked in Linear
project **C++ Concurrency Mastery**.

## How to use
1. Pick the current chapter from the table.
2. Read its `README.md` (concepts, mini-project, katas, quiz).
3. Implement in `practice.cpp`/`practice.hpp`, write tests in `test_practice.cpp`.
4. Build and run the test suite (see Setup); answer the self-quiz from memory.
5. Check the box below and close the Linear issue when **tests pass + quiz answered**.

## Setup
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build --output-on-failure
```

### With ThreadSanitizer (recommended for Ch05+ once you're touching atomics/lock-free code)
```bash
cmake -S . -B build-tsan -DCMAKE_BUILD_TYPE=Debug -DENABLE_TSAN=ON
cmake --build build-tsan -j
ctest --test-dir build-tsan --output-on-failure
```

## Progress
| Ch | Title | Part | Track | Done |
|---:|-------|------|-------|:----:|
| 1 | Hello, World of Concurrency in C++! | Part I | refresher | ✅ |
| 2 | Managing Threads | Part I | refresher | ✅ |
| 3 | Sharing Data Between Threads | Part II | refresher | ☐ |
| 4 | Synchronizing Concurrent Operations | Part II | refresher | ☐ |
| 5 | The C++ Memory Model and Operations on Atomic Types | Part III | full | ☐ |
| 6 | Designing Lock-Based Concurrent Data Structures | Part IV | full | ☐ |
| 7 | Designing Lock-Free Concurrent Data Structures | Part IV | full | ☐ |
| 8 | Designing Concurrent Code | Part IV | full | ☐ |
| 9 | Advanced Thread Management | Part V | full | ☐ |
| 10 | Parallel Algorithms | Part V | full | ☐ |
| 11 | Testing and Debugging Multithreaded Applications | Part VI | full | ☐ |

*Tracks: **full** = new material (read+practice+quiz), **refresher** = already read (quick kata+quiz to confirm).*
