#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <vector>

using namespace std;

struct Item {
    int id;
    long long value;
    long long weight;
};

class KnapsackLocalSearchSolver {
public:
    KnapsackLocalSearchSolver(int item_count, long long capacity, vector<Item> items)
        : n_(item_count), capacity_(capacity), items_(std::move(items)), rng_(42) {}

    double Solve() {
        const auto start_time = chrono::steady_clock::now();
        BuildInitialSolution();
        LocalSearch();
        const auto end_time = chrono::steady_clock::now();
        const chrono::duration<double> elapsed = end_time - start_time;
        PrintAnswer();
        return elapsed.count();
    }

private:
    int n_;
    long long capacity_;
    vector<Item> items_;
    mt19937 rng_;

    vector<char> in_solution_;
    vector<int> chosen_;
    vector<int> not_chosen_;
    long long current_value_ = 0;
    long long current_weight_ = 0;

    vector<char> best_solution_;
    long long best_value_ = 0;
    long long best_weight_ = 0;

    static bool BetterByRatio(const Item& a, const Item& b) {
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

    void RebuildLists() {
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

    void SaveBest() {
        if (current_value_ > best_value_ ||
            (current_value_ == best_value_ && current_weight_ < best_weight_)) {
            best_value_ = current_value_;
            best_weight_ = current_weight_;
            best_solution_ = in_solution_;
        }
    }

    void BuildInitialSolution() {
        in_solution_.assign(n_, 0);
        vector<int> order(n_);
        iota(order.begin(), order.end(), 0);
        sort(order.begin(), order.end(), [&](int lhs, int rhs) {
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

    bool TryAddMove() {
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

    bool TrySwapMove() {
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

    void RandomRestart() {
        vector<int> order(n_);
        iota(order.begin(), order.end(), 0);
        shuffle(order.begin(), order.end(), rng_);

        in_solution_.assign(n_, 0);
        current_value_ = 0;
        current_weight_ = 0;

        for (int id : order) {
            if (uniform_int_distribution<int>(0, 99)(rng_) < 55 &&
                current_weight_ + items_[id].weight <= capacity_) {
                in_solution_[id] = 1;
                current_weight_ += items_[id].weight;
                current_value_ += items_[id].value;
            }
        }

        vector<int> greedy_tail = order;
        sort(greedy_tail.begin(), greedy_tail.end(), [&](int lhs, int rhs) {
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

    void LocalSearch() {
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

    void PrintAnswer() const {
        cout << current_value_ << '\n';
        for (int i = 0; i < n_; ++i) {
            cout << (in_solution_[i] ? 1 : 0);
            if (i + 1 < n_) {
                cout << ' ';
            }
        }
        cout << '\n';
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    long long capacity;
    if (!(cin >> n >> capacity)) {
        return 0;
    }

    vector<Item> items;
    items.reserve(n);
    for (int i = 0; i < n; ++i) {
        long long value, weight;
        cin >> value >> weight;
        items.push_back({i, value, weight});
    }

    KnapsackLocalSearchSolver solver(n, capacity, std::move(items));
    const double elapsed_seconds = solver.Solve();
    cout << "Time: " << fixed << setprecision(6) << elapsed_seconds << '\n';
    return 0;
}
