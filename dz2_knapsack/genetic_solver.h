#pragma once

#include "knapsack_instance.h"
#include "knapsack_solution.h"
#include <random>
#include <vector>

class GeneticKnapsackSolver {
public:
    explicit GeneticKnapsackSolver(
        const KnapsackInstance& instance,
        int population_size = 120,
        int generations = 400,
        double mutation_probability = 0.02,
        int tournament_size = 3,
        int elite_count = 2,
        std::uint32_t seed = 42);

    KnapsackSolution Solve();

private:
    struct Individual {
        std::vector<char> taken;
        long long value = 0;
        long long weight = 0;
    };

    const KnapsackInstance& instance_;
    int population_size_;
    int generations_;
    double mutation_probability_;
    int tournament_size_;
    int elite_count_;
    std::mt19937 rng_;
    std::vector<int> ratio_order_;

    static bool BetterByRatio(const Item& a, const Item& b);
    void BuildRatioOrder();
    void Evaluate(Individual& individual) const;
    void RepairAndFill(Individual& individual) const;
    Individual RandomIndividual();
    int TournamentSelect(const std::vector<Individual>& population);
    Individual Crossover(const Individual& first, const Individual& second);
    void Mutate(Individual& individual);
};
