#include "practice.hpp"

// Ad-hoc manual run of Chapter 04 katas — separate from the GoogleTest target
// so this file's `main()` never conflicts with `gtest_main`.

#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <print>
#include <vector>

int main() {
    // Kata 1 : Missed wake up when cv is waiting without a predicate
    std::mutex m;
    std::condition_variable cv;
    bool ready = false;

    std::thread t1([&cv, &m, &ready] {
        std::unique_lock<std::mutex> lock(m);
        ready = true;
        cv.notify_one();
    });

    std::thread t2([&cv, &m, &ready] {
        std::unique_lock<std::mutex> lock(m);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // cv.wait(lock);  // missed wake up: t1 notified before t2 started waiting
        cv.wait(lock, [&ready] { return ready;});
    });

    t1.join();
    t2.join();

    // Kata 2 : fan out several independent async tasks and wait for all of them to finish
    std::vector<int> limits = {200000, 200000, 200000, 200000, 200000, 200000, 200000, 200000};

    auto seq_start = std::chrono::steady_clock::now();
    auto seq_results = count_primes_sequential(limits);
    auto seq_elapsed = std::chrono::steady_clock::now() - seq_start;

    auto par_start = std::chrono::steady_clock::now();
    auto par_results = count_primes_parallel_async(limits);
    auto par_elapsed = std::chrono::steady_clock::now() - par_start;

    std::println("sequential: {} ms (result[0] = {})",
                  std::chrono::duration_cast<std::chrono::milliseconds>(seq_elapsed).count(),
                  seq_results[0]);
    std::println("parallel (async):  {} ms (result[0] = {})",
                  std::chrono::duration_cast<std::chrono::milliseconds>(par_elapsed).count(),
                  par_results[0]);

    return 0;
}
