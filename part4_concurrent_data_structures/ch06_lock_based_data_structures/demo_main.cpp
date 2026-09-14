#include "practice.hpp"

#include <print>
#include <thread>

// Ad-hoc manual run of Chapter 06 kata 2 — separate from the GoogleTest
// target so this file's `main()` never conflicts with `gtest_main`.

namespace {

// Symmetric producer/consumer counts to sweep, from uncontended up to well
// past this machine's hardware_concurrency().
constexpr int kThreadCounts[] = {1, 2, 4, 6, 8, 12, 16, 24, 32, 48, 64};

// Held constant across configs so every row measures the same total work -
// throughput (not raw elapsed time) is what's comparable across thread counts.
constexpr long long kTotalItems = 2'000'000;

double throughput(long long total_items, long long elapsed_ms) {
    if (elapsed_ms == 0) return 0.0;
    return static_cast<double>(total_items) / (static_cast<double>(elapsed_ms) / 1000.0);
}

}  // namespace

int main() {
    unsigned int cores = std::thread::hardware_concurrency();
    std::println("hardware_concurrency: {}", cores);
    std::println("\nKata 2: fine-grained-locking queue vs. single-mutex queue");
    std::println("({} total items pushed/popped per run, split evenly across producers)\n", kTotalItems);

    std::println("{:>10} {:>14} {:>14} {:>14} {:>14} {:>10}",
                  "threads", "single-mtx ms", "single-mtx/s", "fine-grain ms", "fine-grain/s", "winner");

    int crossover_threads = -1;
    for (int threads : kThreadCounts) {
        long long items_per_producer = kTotalItems / threads;

        long long single_ms = benchmark_queue<SingleMutexQueue<int>>(threads, threads, items_per_producer);
        long long fine_ms = benchmark_queue<FineGrainedQueue<int>>(threads, threads, items_per_producer);

        double single_tp = throughput(kTotalItems, single_ms);
        double fine_tp = throughput(kTotalItems, fine_ms);

        const char* winner = fine_tp > single_tp ? "fine-grain" : "single-mtx";
        if (crossover_threads == -1 && fine_tp > single_tp) {
            crossover_threads = threads;
        }

        std::println("{:>10} {:>14} {:>14.0f} {:>14} {:>14.0f} {:>10}",
                      threads, single_ms, single_tp, fine_ms, fine_tp, winner);
    }

    std::println("");
    if (crossover_threads != -1) {
        std::println("Crossover: fine-grained overtakes single-mutex at {} producers / {} consumers.",
                      crossover_threads, crossover_threads);
    } else {
        std::println("No crossover observed in this range — single-mutex won at every thread count tried.");
    }

    return 0;
}
