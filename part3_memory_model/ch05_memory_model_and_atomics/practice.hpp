// Chapter 05: The C++ Memory Model and Operations on Atomic Types — practice.
//
// Mini-project: Implement a spinlock using `std::atomic_flag` with correct acquire (on lock) / release (on unlock) ordering. Benchmark it against `std::mutex` under light and heavy contention.
//
// Declare your types/functions here; implement in practice.cpp.

#ifndef CH05_MEMORY_MODEL_AND_ATOMICS_PRACTICE_HPP
#define CH05_MEMORY_MODEL_AND_ATOMICS_PRACTICE_HPP

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

class Spinlock {
public:
    void lock() {
        while (flag.test_and_set(std::memory_order_acquire)) {
            // busy-wait
        }
    }

    void unlock() {
        flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

// Test-and-test-and-set (TTAS) variant: spin on a cheap, cacheable plain
// read first, and only attempt the expensive atomic read-modify-write
// (test_and_set) once that plain read suggests the lock is actually free.
// The naive Spinlock above calls test_and_set() on every single spin
// iteration - even the failing ones - which forces cache-coherence traffic
// (invalidating other cores' cached copy of that line) regardless of
// success. Under sustained contention that constant RMW ping-ponging can
// get expensive; TTAS spins on a read that can be satisfied from a locally
// cached copy instead, only paying for the RMW when a lock attempt is
// actually likely to succeed.
class SpinlockTTAS {
public:
    void lock() {
        while (true) {
            while (flag.test(std::memory_order_relaxed)) {
                // cheap plain read, no RMW - doesn't force cache-line traffic
            }
            if (!flag.test_and_set(std::memory_order_acquire)) {
                return;  // acquired
            }
            // someone else grabbed it between our test() and test_and_set() - retry
        }
    }

    void unlock() {
        flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

// Benchmark: num_threads threads each increment a shared counter
// iterations_per_thread times, each increment under Mutex's lock/unlock.
// Returns elapsed wall time in milliseconds.
template <typename Mutex>
long long benchmark_lock(int num_threads, long long iterations_per_thread) {
    Mutex mtx;
    long long counter = 0;

    auto worker = [&mtx, &counter, iterations_per_thread] {
        for (long long i = 0; i < iterations_per_thread; ++i) {
            std::lock_guard<Mutex> lock(mtx);
            ++counter;
        }
    };

    auto start = std::chrono::steady_clock::now();

    std::vector<std::thread> threads;
    threads.reserve(static_cast<size_t>(num_threads));
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker);
    }
    for (auto& t : threads) {
        t.join();
    }

    auto elapsed = std::chrono::steady_clock::now() - start;
    return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
}

// Kata 1: single-producer/single-consumer handoff via acquire/release.
//
// `data` is a plain (non-atomic) int. Correctness depends entirely on the
// acquire/release pairing on `ready`: release on the store guarantees every
// write that happened-before it (here, the write to `data`) is visible to
// any thread whose acquire-load observes `true`; acquire on the load is
// what actually establishes that happens-before edge on the consumer side.
struct SpscHandoff {
    int data = 0;
    std::atomic<bool> ready{false};

    void produce(int value) {
        data = value;                                   // plain write
        ready.store(true, std::memory_order_release);    // publish
    }

    int consume() {
        while (!ready.load(std::memory_order_acquire)) {
            // spin
        }
        return data;  // guaranteed to see produce()'s write to `data`
    }
};

// Same shape, deliberately weakened to memory_order_relaxed on both sides.
// Relaxed gives no happens-before edge between the store to `ready` and the
// plain write to `data` - a consumer that observes ready == true has NO
// guarantee it will also see the producer's write to `data`. The two
// writes (and the two reads) can each be reordered independently as far as
// the other thread is concerned. This isn't just a theoretical concern:
// `data` is a plain non-atomic variable written by one thread and read by
// another with no synchronization edge between them under relaxed ordering
// - that is a genuine data race per the standard, confirmed with
// ThreadSanitizer (see demo_main.cpp).
struct SpscHandoffRelaxed {
    int data = 0;
    std::atomic<bool> ready{false};

    void produce(int value) {
        data = value;
        ready.store(true, std::memory_order_relaxed);
    }

    int consume() {
        while (!ready.load(std::memory_order_relaxed)) {
            // spin
        }
        return data;  // NOT guaranteed to observe produce()'s write
    }
};

// Kata 2: intrusive reference counter mirroring shared_ptr's control block.
//
// add_ref() only needs relaxed ordering: the calling thread already holds a
// valid reference (that's the only way it could be calling add_ref() at
// all), so there's no new data being published to other threads here -
// just an atomic increment that must not lose updates under contention.
//
// release() uses release ordering on the decrement: it guarantees every
// write this thread made while it held its reference happens-before the
// decrement, so whichever thread's decrement is the one that drops the
// count to zero can see them - IF that thread also does an acquire.
//
// That's the subtle part reproduced from real shared_ptr implementations:
// a single acquire operation, executed only by the thread whose fetch_sub
// sees the count go 1 -> 0, is enough to synchronize-with EVERY prior
// release decrement, not just the most recent one. That works because
// fetch_sub is a read-modify-write: it participates in count_'s total
// modification order, so the final decrement's fetch_sub necessarily reads
// the result of every earlier decrement, and the acquire extends that into
// a full happens-before relationship with all of their release writes too.
// Without it (or if release() used relaxed throughout), the destroying
// thread would have no such guarantee, and could destroy the object while
// still missing writes another thread made just before its own release().
//
// The kata allows either an acquire fence or an acquire read here - this
// uses an extra acquire load on count_ rather than a standalone
// atomic_thread_fence. Both are equally correct per the standard, but they
// were NOT equally easy to verify: a standalone-fence version of this
// exact pattern produced a ThreadSanitizer false positive on this machine
// (Homebrew LLVM clang++ + libc++) - a known real limitation, standalone
// atomic_thread_fence is harder for TSan's happens-before graph to model
// precisely than acquire/release attached directly to the atomic
// operation. Swapping to an acquire load on the same variable made the
// warning disappear across repeated runs with identical logic otherwise.
class IntrusiveRefCounted {
public:
    void add_ref() {
        count_.fetch_add(1, std::memory_order_relaxed);
    }

    // Returns true if this call dropped the count to zero - the caller
    // (and only the caller of the release() that returns true) is
    // responsible for destroying the object.
    bool release() {
        if (count_.fetch_sub(1, std::memory_order_release) == 1) {
            count_.load(std::memory_order_acquire);
            return true;
        }
        return false;
    }

    int use_count() const {
        return count_.load(std::memory_order_relaxed);
    }

private:
    std::atomic<int> count_{1};  // starts at 1 for the initial owner
};

#endif // CH05_MEMORY_MODEL_AND_ATOMICS_PRACTICE_HPP
