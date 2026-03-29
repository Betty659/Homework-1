#pragma once

#include "knapsack_instance.h"
#include "knapsack_solution.h"
#include <vector>

class BranchAndBoundKnapsackSolver {
public:
    explicit BranchAndBoundKnapsackSolver(const KnapsackInstance& instance);
    KnapsackSolution Solve();

private:
    const KnapsackInstance& instance_;
    std::vector<int> sorted_ids_;
    std::vector<char> current_taken_;
    std::vector<char> best_taken_;
    long long best_value_ = 0;
    long long best_weight_ = 0;

    static bool BetterByRatio(const Item& a, const Item& b);
    double FractionalUpperBound(int pos, long long current_weight, long long current_value) const;
    void Dfs(int pos, long long current_weight, long long current_value);
};
