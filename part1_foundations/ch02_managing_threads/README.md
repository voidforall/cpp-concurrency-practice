# Chapter 02: Managing Threads

**Part I — Foundations** · Track: **REFRESHER (already read — confirm mastery)**

## Key concepts
- Passing arguments to threads (always copied/moved, not implicitly referenced)
- Dangling references when a detached thread outlives a local variable it captured by reference
- Transferring ownership of a thread (`std::move`, threads are move-only)
- `std::thread::hardware_concurrency()`
- Identifying threads via `get_id()`
- C++20 `std::jthread` and cooperative cancellation via `stop_token`

## Mini-project
Write a `parallel_for(first, last, f)` that partitions a range across `hardware_concurrency()` worker threads, each processing a contiguous chunk, then joins all of them.

## Katas
1. Deliberately capture a local variable by reference in a detached thread's lambda so it dangles after the function returns (observe the bug, e.g. with ASan); fix it by capturing by value.
2. Rewrite `parallel_for` using `std::jthread` and a `std::stop_token` so the caller can cooperatively cancel remaining work early.

## Self-quiz (answer without notes)
1. Why does `std::thread`'s constructor copy/move arguments into internal storage rather than binding by reference, even when the target function takes a reference parameter?

   **A:** Because the thread may keep running after the calling scope returns — especially once detached, or even just before `join()` — so binding directly to the caller's original arguments would risk a dangling reference by default. Copying/moving decouples the argument's lifetime from the caller's stack frame. Reference semantics have to be opted into explicitly:

   ```cpp
   void increment(int& x) { ++x; }

   int counter = 0;
   std::thread t1(increment, counter);            // copies counter; t1 mutates its own copy, `counter` unaffected
   std::thread t2(increment, std::ref(counter));   // explicit opt-in: t2 mutates the real `counter`
   ```

2. What UB can result from `detach()`-ing a thread that captured a stack-local variable by reference?

   **A:** A dangling reference into memory that's already been destroyed/reused once the enclosing function returns — demonstrated concretely in kata 1's `demo_main.cpp`:

   ```cpp
   void spawn_dangling_thread() {
       int x = 42;
       std::thread t([&x] {
           while (true) { std::println("x = {}", x); /* ... */ }
       });
       t.detach();
   }  // x's storage is gone the instant this function returns
   ```

   UB means "anything could happen," not a fixed wrong answer — in our actual run it consistently printed garbage (`x = 1`) instead of crashing, and the exact garbage value even shifted mid-run as that stack memory got reused elsewhere.

3. How does `std::jthread` differ from `std::thread` beyond auto-join in its destructor?

   **A:** Two things beyond auto-join: (a) if still joinable, `jthread`'s destructor calls `request_stop()` **before** joining — a cooperative stop request, not just a blocking cleanup — and (b) if the callable's first parameter accepts a `std::stop_token`, `jthread` automatically injects its own internal token as that argument:

   ```cpp
   std::jthread t([](std::stop_token st) {
       while (!st.stop_requested()) { /* do work */ }
   });
   // t's destructor: request_stop() (sets st.stop_requested() == true), then join() —
   // no manual signaling needed for this simple single-thread case.
   ```

   Note: `parallel_for`'s cancellable overload deliberately does *not* rely on this auto-injection, since each `jthread`'s own internal token would be unrelated to the others — it needs one shared token across every worker, supplied externally instead.

## Definition of done
- [x] Kata(s) run & test suite passes
- [x] Self-quiz answered from memory

---
*Put declarations in `practice.hpp`, implementation in `practice.cpp`, tests in `test_practice.cpp`.
Build with CMake from the repo root (see top-level README).*
