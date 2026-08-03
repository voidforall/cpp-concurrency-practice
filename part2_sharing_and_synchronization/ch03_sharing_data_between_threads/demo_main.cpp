#include "practice.hpp"

#include <thread>
#include <mutex>
#include <vector>
#include <print>

// Ad-hoc manual run of Chapter 03 katas — separate from the GoogleTest target
// so this file's `main()` never conflicts with `gtest_main`.

void run_kata2_naive_singleton_race() {
    // Spawns many threads hammering the naive double-checked-locking
    // singleton concurrently. On a normal build this will very likely
    // "look" fine - that's the trap. This project's -DENABLE_TSAN=ON
    // (g++-15) doesn't link on this machine - Homebrew GCC's TSan runtime
    // isn't available here. Verified instead with Homebrew LLVM's clang++
    // + libc++, which does have a working TSan runtime:
    //
    //   /opt/homebrew/opt/llvm/bin/clang++ -std=c++20 -stdlib=libc++ -pthread \
    //     -fsanitize=thread -g -O1 -I<this dir> demo_main.cpp -o /tmp/ch03_tsan_demo
    //   /tmp/ch03_tsan_demo
    //
    // (confirmed: TSan flags a real data race between the unsynchronized
    // read at practice.hpp's `if (instance == nullptr)` and the locked
    // write `instance = new LazySingleton()`, exactly as expected)
    constexpr int num_threads = 32;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([] {
            LazySingleton* instance = get_instance_naive();
            (void)instance;
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    std::println("naive singleton value: {}", get_instance_naive()->value());
}

int main() {
    run_kata2_naive_singleton_race();
    // deadlock from two threads acquiring two mutexes in opposite order
    std::mutex mutex1, mutex2;
    // auto thread1 = std::thread([&]() {
    //     std::lock_guard<std::mutex> lock1(mutex1);
    //     std::this_thread::sleep_for(std::chrono::milliseconds(100));
    //     std::lock_guard<std::mutex> lock2(mutex2);
    // });

    // auto thread2 = std::thread([&]() {
    //     std::lock_guard<std::mutex> lock2(mutex2);
    //     std::this_thread::sleep_for(std::chrono::milliseconds(100));
    //     std::lock_guard<std::mutex> lock1(mutex1);
    // });

    auto thread1 = std::thread([&]() {
        std::scoped_lock lock(mutex1, mutex2);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

    auto thread2 = std::thread([&]() {
        std::scoped_lock lock(mutex1, mutex2);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

    thread1.join();
    thread2.join();

    return 0;
}
