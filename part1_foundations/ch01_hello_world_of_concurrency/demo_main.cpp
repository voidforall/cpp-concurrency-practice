#include "practice.hpp"

// Ad-hoc manual run of Chapter 01 katas — separate from the GoogleTest target
// so this file's `main()` never conflicts with `gtest_main`.

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int num_threads = 3;
    std::vector<int> partial_sums;
    calculate_partial_sum(numbers, num_threads, partial_sums);

    return 0;
}
