# Chapter 06: Designing Lock-Based Concurrent Data Structures

**Part IV — Concurrent Data Structures & Code Design** · Track: **FULL (new material — read, practice, quiz)**

## Key concepts
- Guidelines for thread-safe interface design (avoiding interface-level races)
- Fine-grained locking vs. a single coarse mutex
- Thread-safe queue with condition_variable + fine-grained locks
- Thread-safe lookup table with per-bucket locking
- Thread-safe singly linked list with hand-over-hand locking

## Mini-project
Implement a thread-safe hash-map-like lookup table using fine-grained per-bucket locking (an array of buckets, each guarded by its own mutex) supporting concurrent `get`/`insert`/`erase` from multiple threads.

## Katas
1. Implement a singly linked list supporting concurrent `push_front` and `remove_if` using hand-over-hand locking (lock node N+1 before releasing node N).
2. Benchmark your fine-grained-locking queue against a single-mutex queue under many-producer/many-consumer load; report the throughput crossover point.

## Kata 2 results — fine-grained vs. single-mutex queue

`FineGrainedQueue` (separate `head_mutex_`/`tail_mutex_`, per the book's
Listing 6.2 design) vs. `SingleMutexQueue` (`std::queue` behind one mutex),
both in `practice.hpp`. `ch06_demo` (`demo_main.cpp`) runs
`benchmark_queue<Queue>` at symmetric producer/consumer counts from 1 to 64,
each run pushing/popping a fixed 2,000,000 `int` total (split evenly across
producers), spinning on `try_pop`. Built Release
(`-DCMAKE_BUILD_TYPE=Release`) on an 8-core Apple Silicon Mac (6 performance
+ 2 efficiency cores); results were stable across repeated runs.

| threads (N producers + N consumers) | single-mutex ms | single-mutex ops/s | fine-grained ms | fine-grained ops/s |
|---:|---:|---:|---:|---:|
|  1 |  69 | 28.99M | 155 | 12.90M |
|  2 | 130 | 15.38M | 265 |  7.55M |
|  4 | 107 | 18.69M | 427 |  4.68M |
|  6 | 105 | 19.05M | 433 |  4.62M |
|  8 |  92 | 21.74M | 354 |  5.65M |
| 12 |  93 | 21.51M | 337 |  5.93M |
| 16 |  98 | 20.41M | 317 |  6.31M |
| 24 | 167 | 11.98M | 298 |  6.71M |
| 32 | 122 | 16.39M | 357 |  5.60M |
| 48 | 127 | 15.75M | 335 |  5.97M |
| 64 | 122 | 16.39M | 350 |  5.71M |

**No crossover in this range — the single-mutex queue wins at every thread
count**, by roughly 2-4x. That's not a bug in the fine-grained queue; it's
two costs of the naive two-lock design that a 4-byte payload doesn't hide:

1. **Allocation.** Every `push` on `FineGrainedQueue` does two heap
   allocations (a new dummy `Node` + a `shared_ptr<T>` control block).
   `SingleMutexQueue`'s `std::queue<int>` is backed by `std::deque`, which
   amortizes allocation across chunks. For an `int` payload, allocation cost
   dwarfs lock cost.
2. **`get_tail()` contends with every push.** `try_pop` locks `tail_mutex_`
   (inside `get_tail()`) just to check for emptiness, on *every* call -
   including the busy-spin calls consumers make while the queue is empty.
   So producers and consumers are still serialized through `tail_mutex_` far
   more often than the "separate head/tail locks" framing suggests; the
   two-lock design's real win (a push and a pop running truly concurrently)
   only pays off once the queue reliably holds several buffered elements, so
   pops stop calling `get_tail()`'s equality check against a moving target.

The gap does narrow as thread count rises (~3.4x at 1 thread down to ~1.9x
at 16-24), consistent with the fine-grained design paying off more under
real contention - it just never overtakes the allocation + `get_tail()` tax
within a practical thread-count range at this payload size on this machine.
A larger/costlier payload (e.g. a `std::string` copy or a heavier struct)
would likely shift this, since allocation overhead would matter
proportionally less relative to the copy itself.

Reproduce:
```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --target ch06_demo -j
./build-release/part4_concurrent_data_structures/ch06_lock_based_data_structures/ch06_demo
```

## Self-quiz (answer without notes)
1. Why does a single coarse-grained mutex around an entire data structure become a scalability bottleneck as thread count grows?
2. What invariant must hand-over-hand locking preserve at every step to stay both correct and deadlock-free?
3. How many buckets/locks would you choose for a fine-grained lookup table, and what's the tradeoff in picking too many vs. too few?

## Definition of done
- [ ] Mini-project + katas run & test suite passes
- [ ] Self-quiz answered from memory

---
*Put declarations in `practice.hpp`, implementation in `practice.cpp`, tests in `test_practice.cpp`.
Build with CMake from the repo root (see top-level README).*
