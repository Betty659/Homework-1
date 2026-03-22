#include "knapsack_solver.h"
#include <iostream>
#include <iomanip>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n;
    long long capacity;
    if (!(std::cin >> n >> capacity)) {
        return 0;
    }

    std::vector<Item> items;
    items.reserve(n);
    for (int i = 0; i < n; ++i) {
        long long value, weight;
        std::cin >> value >> weight;
        items.push_back({i, value, weight});
    }

    KnapsackLocalSearchSolver solver(n, capacity, std::move(items));
    const double elapsed_seconds = solver.Solve();
    std::cout << "Time: " << std::fixed << std::setprecision(6) << elapsed_seconds << '\n';
    return 0;
}