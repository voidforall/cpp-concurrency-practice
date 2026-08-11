#include "practice.hpp"

#include <array>
#include <atomic>
#include <mutex>
#include <print>
#include <thread>
#include <vector>

// Ad-hoc manual run of Chapter 05 katas — separate from the GoogleTest target
// so this file's `main()` never conflicts with `gtest_main`.

namespace {

constexpr int kNumOwners = 16;

// Test fixture for kata 2, not a general-purpose type - IntrusiveRefCounted
// itself (the actual kata deliverable) lives in practice.hpp.
struct RefCountedResource : IntrusiveRefCounted {
    std::array<int, kNumOwners> slots{};
    static inline std::atomic<int> destroy_count{0};

    ~RefCountedResource() {
        destroy_count.fetch_add(1, std::memory_order_relaxed);
    }
};

}  // namespace

int main() {
    unsigned int cores = std::thread::hardware_concurrency();
    std::println("hardware_concurrency: {}", cores);

    // Light contention: few threads, so no CPU oversubscription - each
    // thread mostly gets to run uncontended, critical section is tiny.
    int light_threads = 2;
    long long light_iters = 2'000'000;

    long long spin_light = benchmark_lock<Spinlock>(light_threads, light_iters);
    long long ttas_light = benchmark_lock<SpinlockTTAS>(light_threads, light_iters);
    long long mutex_light = benchmark_lock<std::mutex>(light_threads, light_iters);
    std::println("\nlight contention ({} threads, {} iters each):", light_threads, light_iters);
    std::println("  Spinlock (naive TAS): {} ms", spin_light);
    std::println("  SpinlockTTAS:         {} ms", ttas_light);
    std::println("  std::mutex:           {} ms", mutex_light);

    // Heavy contention: many more threads than cores, all hammering the
    // same lock - spinning threads now compete for real CPU time against
    // the thread that actually holds the lock.
    int heavy_threads = static_cast<int>(cores) * 4;
    long long heavy_iters = 200'000;

    long long spin_heavy = benchmark_lock<Spinlock>(heavy_threads, heavy_iters);
    long long ttas_heavy = benchmark_lock<SpinlockTTAS>(heavy_threads, heavy_iters);
    long long mutex_heavy = benchmark_lock<std::mutex>(heavy_threads, heavy_iters);
    std::println("\nheavy contention ({} threads, {} iters each):", heavy_threads, heavy_iters);
    std::println("  Spinlock (naive TAS): {} ms", spin_heavy);
    std::println("  SpinlockTTAS:         {} ms", ttas_heavy);
    std::println("  std::mutex:           {} ms", mutex_heavy);

    // Kata 1: SPSC handoff, acquire/release vs relaxed.
    //
    // Run many trials of each. The acquire/release version must be correct
    // on every single trial - that's not optional, it's what the ordering
    // guarantees. The relaxed version is genuine UB (a data race on `data`),
    // and it does in fact show 0/100000 failures on this hardware - a UB
    // race doesn't have to visibly misbehave to be real (same lesson as
    // Ch03's naive double-checked-locking singleton). This project's
    // -DENABLE_TSAN=ON (g++-15) doesn't link on this machine - see the
    // memory note - so it was verified instead with Homebrew LLVM's
    // clang++ + libc++, which does have a working TSan runtime:
    //
    //   /opt/homebrew/opt/llvm/bin/clang++ -std=c++20 -stdlib=libc++ -pthread \
    //     -fsanitize=thread -g -O1 -I<this dir> <a small repro> -o /tmp/spsc_tsan
    //   /tmp/spsc_tsan
    //
    // Confirmed: TSan flags a real data race on SpscHandoffRelaxed - a
    // 4-byte read (consume()) racing a 4-byte write (produce()), matching
    // `data`'s size exactly; `ready` itself is never flagged, since it's
    // the plain `data` access that has no happens-before edge under
    // relaxed ordering. SpscHandoff (acquire/release) is TSan-clean under
    // the identical test.
    constexpr int trials = 100'000;

    int correct_failures = 0;
    for (int i = 0; i < trials; ++i) {
        SpscHandoff h;
        std::thread producer([&h, i] { h.produce(i); });
        int result = h.consume();
        producer.join();
        if (result != i) {
            ++correct_failures;
        }
    }
    std::println("\nSpscHandoff (acquire/release): {} / {} trials incorrect", correct_failures, trials);

    int relaxed_failures = 0;
    for (int i = 0; i < trials; ++i) {
        SpscHandoffRelaxed h;
        std::thread producer([&h, i] { h.produce(i); });
        int result = h.consume();
        producer.join();
        if (result != i) {
            ++relaxed_failures;
        }
    }
    std::println("SpscHandoffRelaxed (relaxed):  {} / {} trials incorrect", relaxed_failures, trials);
    std::println("(0 failures here doesn't mean it's correct - see the comment above for the TSan proof)");

    // Kata 2: intrusive ref counting stress test.
    //
    // Whichever release() call happens to drop the count to zero must be
    // able to see every other thread's write to its own slot - but the
    // ONLY synchronization connecting sibling owner threads to each other
    // here is the ref counter itself (they never join each other, and the
    // payload check happens inside the winning thread's own lambda, before
    // main ever joins anyone - so this can't accidentally piggyback on
    // std::thread::join()'s own happens-before guarantee).
    auto* res = new RefCountedResource();  // count = 1 (main's own reference)
    for (int i = 0; i < kNumOwners; ++i) {
        res->add_ref();  // count = 1 + kNumOwners
    }

    std::atomic<bool> destroyed{false};
    std::atomic<int> observed_sum{-1};

    auto try_release_and_verify = [&](RefCountedResource* r) {
        if (r->release()) {
            int sum = 0;
            for (int v : r->slots) {
                sum += v;
            }
            observed_sum.store(sum, std::memory_order_relaxed);
            destroyed.store(true, std::memory_order_relaxed);
            delete r;
        }
    };

    std::vector<std::thread> owners;
    for (int i = 0; i < kNumOwners; ++i) {
        owners.emplace_back([res, i, &try_release_and_verify] {
            res->slots[i] = i + 1;
            try_release_and_verify(res);
        });
    }

    // main's own original reference races too - almost certainly resolves
    // near-instantly (before owner threads even get scheduled), so in
    // practice one of the owner threads ends up being the actual last
    // releaser, but which one is genuinely unpredictable.
    try_release_and_verify(res);

    for (auto& t : owners) {
        t.join();
    }

    constexpr int expected_sum = kNumOwners * (kNumOwners + 1) / 2;
    std::println("\nKata 2: IntrusiveRefCounted stress test ({} owners)", kNumOwners);
    std::println("  destroyed:      {}", destroyed.load());
    std::println("  destroy_count:  {} (must be exactly 1 - no double-free, no leak)",
                  RefCountedResource::destroy_count.load());
    std::println("  observed sum:   {} (expected {})", observed_sum.load(), expected_sum);

    return 0;
}
