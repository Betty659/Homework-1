#include "tsp_solver.h"
#include <iostream>
#include <iomanip>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n;
    if (!(std::cin >> n)) {
        return 0;
    }

    std::vector<Point> points(n);
    for (int i = 0; i < n; ++i) {
        std::cin >> points[i].x >> points[i].y;
    }

    TSPLocalSearchSolver solver(std::move(points));
    const double elapsed_seconds = solver.Solve();
    std::cout << "Time: " << std::fixed << std::setprecision(6) << elapsed_seconds << '\n';
    return 0;
}