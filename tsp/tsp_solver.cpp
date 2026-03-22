#include "tsp_solver.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>

TSPLocalSearchSolver::TSPLocalSearchSolver(std::vector<Point> points) : points_(std::move(points)) {
    n_ = static_cast<int>(points_.size());
}

double TSPLocalSearchSolver::Solve() {
    const auto start_time = std::chrono::steady_clock::now();
    BuildInitialTour();
    ImproveByTwoOpt();
    const auto end_time = std::chrono::steady_clock::now();
    const std::chrono::duration<double> elapsed = end_time - start_time;
    PrintAnswer();
    return elapsed.count();
}

double TSPLocalSearchSolver::Dist(int a, int b) const {
    double dx = points_[a].x - points_[b].x;
    double dy = points_[a].y - points_[b].y;
    return std::sqrt(dx * dx + dy * dy);
}

double TSPLocalSearchSolver::TourLength(const std::vector<int>& order) const {
    if (order.empty()) {
        return 0.0;
    }

    double total = 0.0;
    for (int i = 0; i < n_; ++i) {
        total += Dist(order[i], order[(i + 1) % n_]);
    }
    return total;
}

void TSPLocalSearchSolver::BuildInitialTour() {
    tour_.resize(n_);
    std::iota(tour_.begin(), tour_.end(), 0);

    std::sort(tour_.begin(), tour_.end(), [&](int lhs, int rhs) {
        if (points_[lhs].x != points_[rhs].x) {
            return points_[lhs].x < points_[rhs].x;
        }
        return points_[lhs].y < points_[rhs].y;
    });

    std::vector<int> reordered;
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

bool TSPLocalSearchSolver::TwoOptPass(int window) {
    bool improved = false;

    for (int i = 0; i + 2 < n_; ++i) {
        int upper = std::min(n_ - 1, i + window);
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
                std::reverse(tour_.begin() + i + 1, tour_.begin() + k + 1);
                best_length_ += delta;
                improved = true;
            }
        }
    }

    return improved;
}

void TSPLocalSearchSolver::ImproveByTwoOpt() {
    if (n_ < 4) {
        best_length_ = TourLength(tour_);
        return;
    }

    const auto start = std::chrono::steady_clock::now();
    const auto time_limit = std::chrono::milliseconds(1500);

    int window = (n_ <= 500) ? n_ - 1 : std::min(80, n_ - 1);
    while (std::chrono::steady_clock::now() - start < time_limit) {
        bool improved = TwoOptPass(window);
        if (!improved) {
            if (window >= n_ - 1) {
                break;
            }
            window = std::min(n_ - 1, window * 2);
        }
    }

    best_length_ = TourLength(tour_);
}

void TSPLocalSearchSolver::PrintAnswer() const {
    std::cout << std::fixed << std::setprecision(6) << best_length_ << '\n';
    for (int i = 0; i < n_; ++i) {
        std::cout << tour_[i];
        if (i + 1 < n_) {
            std::cout << ' ';
        }
    }
    std::cout << '\n';
}