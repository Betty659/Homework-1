#pragma once

#include "item.h"
#include <vector>
#include <random>

class KnapsackLocalSearchSolver {
public:
    KnapsackLocalSearchSolver(int item_count, long long capacity, std::vector<Item> items);

    double Solve();

private:
    int n_;
    long long capacity_;
    std::vector<Item> items_;
    std::mt19937 rng_;

    std::vector<char> in_solution_;
    std::vector<int> chosen_;
    std::vector<int> not_chosen_;
    long long current_value_ = 0;
    long long current_weight_ = 0;

    std::vector<char> best_solution_;
    long long best_value_ = 0;
    long long best_weight_ = 0;

    bool BetterByRatio(const Item& a, const Item& b);
    void RebuildLists();
    void SaveBest();
    void BuildInitialSolution();
    bool TryAddMove();
    bool TrySwapMove();
    void RandomRestart();
    void LocalSearch();
    void PrintAnswer() const;
};