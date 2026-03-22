#include "knapsack_solver.h"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>

KnapsackLocalSearchSolver::KnapsackLocalSearchSolver(int item_count, long long capacity, std::vector<Item> items)
    : n_(item_count), capacity_(capacity), items_(std::move(items)), rng_(42) {}

double KnapsackLocalSearchSolver::Solve() {
    const auto start_time = std::chrono::steady_clock::now();
    BuildInitialSolution();
    LocalSearch();
    const auto end_time = std::chrono::steady_clock::now();
    const std::chrono::duration<double> elapsed = end_time - start_time;
    PrintAnswer();
    return elapsed.count();
}

bool KnapsackLocalSearchSolver::BetterByRatio(const Item& a, const Item& b) {
    if (a.weight == 0 || b.weight == 0) {
        if (a.weight == 0 && b.weight == 0) {
            return a.value > b.value;
        }
        return a.weight == 0;
    }
    long double left = static_cast<long double>(a.value) / static_cast<long double>(a.weight);
    long double right = static_cast<long double>(b.value) / static_cast<long double>(b.weight);
    if (left != right) {
        return left > right;
    }
    if (a.value != b.value) {
        return a.value > b.value;
    }
    return a.weight < b.weight;
}

void KnapsackLocalSearchSolver::RebuildLists() {
    chosen_.clear();
    not_chosen_.clear();
    for (int i = 0; i < n_; ++i) {
        if (in_solution_[i]) {
            chosen_.push_back(i);
        } else {
            not_chosen_.push_back(i);
        }
    }
}

void KnapsackLocalSearchSolver::SaveBest() {
    if (current_value_ > best_value_ ||
        (current_value_ == best_value_ && current_weight_ < best_weight_)) {
        best_value_ = current_value_;
        best_weight_ = current_weight_;
        best_solution_ = in_solution_;
    }
}

void KnapsackLocalSearchSolver::BuildInitialSolution() {
    in_solution_.assign(n_, 0);
    std::vector<int> order(n_);
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](int lhs, int rhs) {
        return BetterByRatio(items_[lhs], items_[rhs]);
    });

    current_value_ = 0;
    current_weight_ = 0;
    for (int id : order) {
        if (current_weight_ + items_[id].weight <= capacity_) {
            in_solution_[id] = 1;
            current_weight_ += items_[id].weight;
            current_value_ += items_[id].value;
        }
    }

    RebuildLists();
    best_solution_ = in_solution_;
    best_value_ = current_value_;
    best_weight_ = current_weight_;
}

bool KnapsackLocalSearchSolver::TryAddMove() {
    int best_item = -1;
    long long best_gain = 0;
    for (int id : not_chosen_) {
        if (current_weight_ + items_[id].weight <= capacity_ && items_[id].value > best_gain) {
            best_gain = items_[id].value;
            best_item = id;
        }
    }

    if (best_item == -1) {
        return false;
    }

    in_solution_[best_item] = 1;
    current_weight_ += items_[best_item].weight;
    current_value_ += items_[best_item].value;
    RebuildLists();
    SaveBest();
    return true;
}

bool KnapsackLocalSearchSolver::TrySwapMove() {
    int best_in = -1;
    int best_out = -1;
    long long best_gain = 0;

    for (int in_id : not_chosen_) {
        for (int out_id : chosen_) {
            long long new_weight = current_weight_ - items_[out_id].weight + items_[in_id].weight;
            long long gain = items_[in_id].value - items_[out_id].value;
            if (new_weight <= capacity_ && gain > best_gain) {
                best_gain = gain;
                best_in = in_id;
                best_out = out_id;
            }
        }
    }

    if (best_in == -1) {
        return false;
    }

    in_solution_[best_out] = 0;
    in_solution_[best_in] = 1;
    current_weight_ = current_weight_ - items_[best_out].weight + items_[best_in].weight;
    current_value_ = current_value_ - items_[best_out].value + items_[best_in].value;
    RebuildLists();
    SaveBest();
    return true;
}

void KnapsackLocalSearchSolver::RandomRestart() {
    std::vector<int> order(n_);
    std::iota(order.begin(), order.end(), 0);
    std::shuffle(order.begin(), order.end(), rng_);

    in_solution_.assign(n_, 0);
    current_value_ = 0;
    current_weight_ = 0;

    for (int id : order) {
        if (std::uniform_int_distribution<int>(0, 99)(rng_) < 55 &&
            current_weight_ + items_[id].weight <= capacity_) {
            in_solution_[id] = 1;
            current_weight_ += items_[id].weight;
            current_value_ += items_[id].value;
        }
    }

    std::vector<int> greedy_tail = order;
    std::sort(greedy_tail.begin(), greedy_tail.end(), [&](int lhs, int rhs) {
        return BetterByRatio(items_[lhs], items_[rhs]);
    });
    for (int id : greedy_tail) {
        if (!in_solution_[id] && current_weight_ + items_[id].weight <= capacity_) {
            in_solution_[id] = 1;
            current_weight_ += items_[id].weight;
            current_value_ += items_[id].value;
        }
    }

    RebuildLists();
    SaveBest();
}

void KnapsackLocalSearchSolver::LocalSearch() {
    const int max_unsuccessful_restarts = 200;
    int unsuccessful_restarts = 0;

    while (unsuccessful_restarts < max_unsuccessful_restarts) {
        bool improved = false;
        const long long best_before_restart = best_value_;

        while (TryAddMove()) {
            improved = true;
        }
        while (TrySwapMove()) {
            improved = true;
            while (TryAddMove()) {
                improved = true;
            }
        }

        if (!improved) {
            RandomRestart();
            if (best_value_ > best_before_restart) {
                unsuccessful_restarts = 0;
            } else {
                ++unsuccessful_restarts;
            }
        }
    }

    in_solution_ = best_solution_;
    current_value_ = best_value_;
    current_weight_ = best_weight_;
    RebuildLists();
}

void KnapsackLocalSearchSolver::PrintAnswer() const {
    std::cout << current_value_ << '\n';
    for (int i = 0; i < n_; ++i) {
        std::cout << (in_solution_[i] ? 1 : 0);
        if (i + 1 < n_) {
            std::cout << ' ';
        }
    }
    std::cout << '\n';
}