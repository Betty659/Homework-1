#include "branch_and_bound_solver.h"
#include <algorithm>
#include <numeric>

BranchAndBoundKnapsackSolver::BranchAndBoundKnapsackSolver(const KnapsackInstance& instance)
    : instance_(instance) {
    sorted_ids_.resize(instance_.item_count);
    std::iota(sorted_ids_.begin(), sorted_ids_.end(), 0);
    std::sort(sorted_ids_.begin(), sorted_ids_.end(), [&](int lhs, int rhs) {
        return BetterByRatio(instance_.items[lhs], instance_.items[rhs]);
    });
}

bool BranchAndBoundKnapsackSolver::BetterByRatio(const Item& a, const Item& b) {
    if (a.weight == 0 || b.weight == 0) {
        if (a.weight == 0 && b.weight == 0) {
            return a.value > b.value;
        }
        return a.weight == 0;
    }

    const long double left = static_cast<long double>(a.value) / static_cast<long double>(a.weight);
    const long double right = static_cast<long double>(b.value) / static_cast<long double>(b.weight);

    if (left != right) {
        return left > right;
    }
    if (a.value != b.value) {
        return a.value > b.value;
    }
    return a.weight < b.weight;
}

double BranchAndBoundKnapsackSolver::FractionalUpperBound(
    int pos,
    long long current_weight,
    long long current_value) const {
    if (current_weight > instance_.capacity) {
        return -1.0;
    }

    long long remaining_capacity = instance_.capacity - current_weight;
    long double bound = static_cast<long double>(current_value);

    for (int i = pos; i < instance_.item_count && remaining_capacity > 0; ++i) {
        const Item& item = instance_.items[sorted_ids_[i]];
        if (item.weight <= remaining_capacity) {
            bound += static_cast<long double>(item.value);
            remaining_capacity -= item.weight;
        } else if (item.weight > 0) {
            const long double fraction =
                static_cast<long double>(remaining_capacity) / static_cast<long double>(item.weight);
            bound += static_cast<long double>(item.value) * fraction;
            remaining_capacity = 0;
        } else {
            bound += static_cast<long double>(item.value);
        }
    }

    return static_cast<double>(bound);
}

void BranchAndBoundKnapsackSolver::Dfs(int pos, long long current_weight, long long current_value) {
    if (current_weight > instance_.capacity) {
        return;
    }

    const double upper_bound = FractionalUpperBound(pos, current_weight, current_value);
    if (upper_bound <= static_cast<double>(best_value_)) {
        return;
    }

    if (pos == instance_.item_count) {
        if (current_value > best_value_ || (current_value == best_value_ && current_weight < best_weight_)) {
            best_value_ = current_value;
            best_weight_ = current_weight;
            best_taken_ = current_taken_;
        }
        return;
    }

    const int item_id = sorted_ids_[pos];
    const Item& item = instance_.items[item_id];

    if (current_weight + item.weight <= instance_.capacity) {
        current_taken_[item_id] = 1;
        Dfs(pos + 1, current_weight + item.weight, current_value + item.value);
        current_taken_[item_id] = 0;
    }

    Dfs(pos + 1, current_weight, current_value);
}

KnapsackSolution BranchAndBoundKnapsackSolver::Solve() {
    current_taken_.assign(instance_.item_count, 0);
    best_taken_.assign(instance_.item_count, 0);
    best_value_ = 0;
    best_weight_ = 0;

    Dfs(0, 0, 0);

    KnapsackSolution result;
    result.value = best_value_;
    result.weight = best_weight_;
    result.taken = best_taken_;
    return result;
}
