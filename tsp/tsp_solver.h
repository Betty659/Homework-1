#pragma once

#include "point.h"
#include <vector>

class TSPLocalSearchSolver {
public:
    explicit TSPLocalSearchSolver(std::vector<Point> points);

    double Solve();

private:
    int n_ = 0;
    std::vector<Point> points_;
    std::vector<int> tour_;
    double best_length_ = 0.0;

    double Dist(int a, int b) const;
    double TourLength(const std::vector<int>& order) const;
    void BuildInitialTour();
    bool TwoOptPass(int window);
    void ImproveByTwoOpt();
    void PrintAnswer() const;
};