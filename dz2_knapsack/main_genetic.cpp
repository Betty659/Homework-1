#include "genetic_solver.h"
#include "knapsack_instance.h"
#include <chrono>
#include <iomanip>
#include <iostream>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    KnapsackInstance instance;
    if (!instance.ReadFromStream(std::cin)) {
        return 0;
    }

    const auto start_time = std::chrono::steady_clock::now();
    GeneticKnapsackSolver solver(instance);
    const KnapsackSolution solution = solver.Solve();
    const auto end_time = std::chrono::steady_clock::now();
    const std::chrono::duration<double> elapsed = end_time - start_time;

    std::cout << solution.value << '\n';
    for (int i = 0; i < instance.item_count; ++i) {
        std::cout << (solution.taken[i] ? 1 : 0);
        if (i + 1 < instance.item_count) {
            std::cout << ' ';
        }
    }
    std::cout << '\n';
    std::cout << "Time: " << std::fixed << std::setprecision(6) << elapsed.count() << '\n';

    return 0;
}
