#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <vector>

using namespace std;

struct Point {
    double x;
    double y;
};

class TSPLocalSearchSolver {
public:
    explicit TSPLocalSearchSolver(vector<Point> points) : points_(std::move(points)) {
        n_ = static_cast<int>(points_.size());
    }

    double Solve() {
        const auto start_time = chrono::steady_clock::now();
        BuildInitialTour();
        ImproveByTwoOpt();
        const auto end_time = chrono::steady_clock::now();
        const chrono::duration<double> elapsed = end_time - start_time;
        PrintAnswer();
        return elapsed.count();
    }

private:
    int n_ = 0;
    vector<Point> points_;
    vector<int> tour_;
    double best_length_ = 0.0;

    double Dist(int a, int b) const {
        double dx = points_[a].x - points_[b].x;
        double dy = points_[a].y - points_[b].y;
        return sqrt(dx * dx + dy * dy);
    }

    double TourLength(const vector<int>& order) const {
        if (order.empty()) {
            return 0.0;
        }

        double total = 0.0;
        for (int i = 0; i < n_; ++i) {
            total += Dist(order[i], order[(i + 1) % n_]);
        }
        return total;
    }

    void BuildInitialTour() {
        tour_.resize(n_);
        iota(tour_.begin(), tour_.end(), 0);

        sort(tour_.begin(), tour_.end(), [&](int lhs, int rhs) {
            if (points_[lhs].x != points_[rhs].x) {
                return points_[lhs].x < points_[rhs].x;
            }
            return points_[lhs].y < points_[rhs].y;
        });

        vector<int> reordered;
        reordered.reserve(n_);
        int left = 0;
        int right = n_ - 1;
        bool take_left = true;
        while (left <= right) {
            if (take_left) {
                reordered.push_back(tour_[left++]);
            } else {
                reordered.push_back(tour_[right--]);
            }
            take_left = !take_left;
        }
        tour_ = std::move(reordered);
        best_length_ = TourLength(tour_);
    }

    bool TwoOptPass(int window) {
        bool improved = false;

        for (int i = 0; i + 2 < n_; ++i) {
            int upper = min(n_ - 1, i + window);
            for (int k = i + 2; k <= upper; ++k) {
                if (i == 0 && k == n_ - 1) {
                    continue;
                }

                int a = tour_[i];
                int b = tour_[i + 1];
                int c = tour_[k];
                int d = tour_[(k + 1) % n_];

                double delta = Dist(a, c) + Dist(b, d) - Dist(a, b) - Dist(c, d);
                if (delta < -1e-12) {
                    reverse(tour_.begin() + i + 1, tour_.begin() + k + 1);
                    best_length_ += delta;
                    improved = true;
                }
            }
        }

        return improved;
    }

    void ImproveByTwoOpt() {
        if (n_ < 4) {
            best_length_ = TourLength(tour_);
            return;
        }

        const auto start = chrono::steady_clock::now();
        const auto time_limit = chrono::milliseconds(1500);

        int window = (n_ <= 500) ? n_ - 1 : min(80, n_ - 1);
        while (chrono::steady_clock::now() - start < time_limit) {
            bool improved = TwoOptPass(window);
            if (!improved) {
                if (window >= n_ - 1) {
                    break;
                }
                window = min(n_ - 1, window * 2);
            }
        }

        best_length_ = TourLength(tour_);
    }

    void PrintAnswer() const {
        cout << fixed << setprecision(6) << best_length_ << '\n';
        for (int i = 0; i < n_; ++i) {
            cout << tour_[i];
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
    if (!(cin >> n)) {
        return 0;
    }

    vector<Point> points(n);
    for (int i = 0; i < n; ++i) {
        cin >> points[i].x >> points[i].y;
    }

    TSPLocalSearchSolver solver(std::move(points));
    const double elapsed_seconds = solver.Solve();
    cout << "Time: " << fixed << setprecision(6) << elapsed_seconds << '\n';
    return 0;
}
