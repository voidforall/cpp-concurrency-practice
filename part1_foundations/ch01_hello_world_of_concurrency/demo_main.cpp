#include "practice.hpp"

#include <thread>
#include <print>
#include <chrono>

int main() {

    // kata 1 : partial sums of a vector of numbers using multiple threads
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int num_threads = 3;
    std::vector<int> partial_sums;
    calculate_partial_sum(numbers, num_threads, partial_sums);

    // kata 2: threads not joined before main() exits, leading to undefined behavior
    // joinable thread t2 leads to UB

    // std::thread t1([](){
    //     std::println("Thread {} is running and will sleep for 2 seconds.", std::this_thread::get_id());
    //     std::this_thread::sleep_for(std::chrono::seconds(2));
    // });

    // std::thread t2([](){
    //     std::println("Thread {} is running and will sleep for 1 second.", std::this_thread::get_id());
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    // });

    // t1.join();

    // Fix with RAII wrapper for std::thread that joins in its destructor
    scoped_thread scoped_t1(std::thread([](){
        std::println("Thread {} is running and will sleep for 2 seconds.", std::this_thread::get_id());
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }));

    scoped_thread scoped_t2(std::thread([](){
        std::println("Thread {} is running and will sleep for 1 second.", std::this_thread::get_id());
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }));
    
    return 0;
}
