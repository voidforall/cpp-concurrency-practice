#include "practice.hpp"

#include <chrono>
#include <print>
#include <thread>

// kata 1: anti-pattern: detached thread capturing a local variable by reference.
//
// If `x` lived directly in main() and main() returned right after detach(),
// the whole process would tear down before the detached thread reliably got
// scheduled even once - the bug would be a coin flip to observe. Separating
// "who owns x" (this function) from "who keeps the process alive" (main)
// makes the dangling read actually reproducible.
void spawn_dangling_thread() {
    int x = 42;

    std::thread t([&x]() {
        while (true) {
            std::println("x = {}", x);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    t.detach();
}  // x's storage is gone the instant this function returns

void good_detached_thread() {
    int x = 42;

    std::thread t([x]() {
        while (true) {
            std::println("x = {}", x);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    t.detach();
}  // x's storage is gone the instant this function returns

int main() {
    spawn_dangling_thread();
    good_detached_thread();
    
    // Keep the process alive so the detached thread has real opportunities
    // to read through the now-dangling reference more than once.
    std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}
