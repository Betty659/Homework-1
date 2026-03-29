#include "genetic_solver.h"
#include <algorithm>
#include <numeric>

GeneticKnapsackSolver::GeneticKnapsackSolver(
    const KnapsackInstance& instance,
    int population_size,
    int generations,
    double mutation_probability,
    int tournament_size,
    int elite_count,
    std::uint32_t seed)
    : instance_(instance),
      population_size_(population_size),
      generations_(generations),
      mutation_probability_(mutation_probability),
      tournament_size_(tournament_size),
      elite_count_(elite_count),
      rng_(seed) {
    BuildRatioOrder();
}

bool GeneticKnapsackSolver::BetterByRatio(const Item& a, const Item& b) {
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

void GeneticKnapsackSolver::BuildRatioOrder() {
    ratio_order_.resize(instance_.item_count);
    std::iota(ratio_order_.begin(), ratio_order_.end(), 0);
    std::sort(ratio_order_.begin(), ratio_order_.end(), [&](int lhs, int rhs) {
        return BetterByRatio(instance_.items[lhs], instance_.items[rhs]);
    });
}

void GeneticKnapsackSolver::Evaluate(Individual& individual) const {
    long long value = 0;
    long long weight = 0;
    for (int i = 0; i < instance_.item_count; ++i) {
        if (individual.taken[i]) {
            value += instance_.items[i].value;
            weight += instance_.items[i].weight;
        }
    }
    individual.value = value;
    individual.weight = weight;
}

void GeneticKnapsackSolver::RepairAndFill(Individual& individual) const {
    Evaluate(individual);

    while (individual.weight > instance_.capacity) {
        int worst_id = -1;
        long double worst_ratio = 1e300L;
        for (int id = 0; id < instance_.item_count; ++id) {
            if (!individual.taken[id]) {
                continue;
            }
            const Item& item = instance_.items[id];
            const long double ratio = (item.weight == 0)
                ? 1e300L
                : static_cast<long double>(item.value) / static_cast<long double>(item.weight);
            if (ratio < worst_ratio) {
                worst_ratio = ratio;
                worst_id = id;
            }
        }
        if (worst_id == -1) {
            break;
        }
        individual.taken[worst_id] = 0;
        individual.value -= instance_.items[worst_id].value;
        individual.weight -= instance_.items[worst_id].weight;
    }

    for (int id : ratio_order_) {
        if (individual.taken[id]) {
            continue;
        }
        const Item& item = instance_.items[id];
        if (individual.weight + item.weight <= instance_.capacity) {
            individual.taken[id] = 1;
            individual.value += item.value;
            individual.weight += item.weight;
        }
    }
}

GeneticKnapsackSolver::Individual GeneticKnapsackSolver::RandomIndividual() {
    Individual candidate;
    candidate.taken.assign(instance_.item_count, 0);
    std::uniform_int_distribution<int> bit_distribution(0, 1);
    for (int i = 0; i < instance_.item_count; ++i) {
        candidate.taken[i] = static_cast<char>(bit_distribution(rng_));
    }
    RepairAndFill(candidate);
    return candidate;
}

int GeneticKnapsackSolver::TournamentSelect(const std::vector<Individual>& population) {
    std::uniform_int_distribution<int> pick(0, static_cast<int>(population.size()) - 1);
    int best_index = pick(rng_);
    for (int i = 1; i < tournament_size_; ++i) {
        const int challenger = pick(rng_);
        if (population[challenger].value > population[best_index].value ||
            (population[challenger].value == population[best_index].value &&
             population[challenger].weight < population[best_index].weight)) {
            best_index = challenger;
        }
    }
    return best_index;
}

GeneticKnapsackSolver::Individual GeneticKnapsackSolver::Crossover(
    const Individual& first,
    const Individual& second) {
    Individual child;
    child.taken.assign(instance_.item_count, 0);
    if (instance_.item_count == 0) {
        return child;
    }

    std::uniform_int_distribution<int> cut_point_distribution(0, instance_.item_count - 1);
    const int cut = cut_point_distribution(rng_);

    for (int i = 0; i < instance_.item_count; ++i) {
        child.taken[i] = (i <= cut) ? first.taken[i] : second.taken[i];
    }

    RepairAndFill(child);
    return child;
}

void GeneticKnapsackSolver::Mutate(Individual& individual) {
    std::uniform_real_distribution<double> chance(0.0, 1.0);
    for (int i = 0; i < instance_.item_count; ++i) {
        if (chance(rng_) < mutation_probability_) {
            individual.taken[i] = static_cast<char>(1 - individual.taken[i]);
        }
    }
    RepairAndFill(individual);
}

KnapsackSolution GeneticKnapsackSolver::Solve() {
    std::vector<Individual> population;
    population.reserve(population_size_);

    for (int i = 0; i < population_size_; ++i) {
        population.push_back(RandomIndividual());
    }

    Individual best = population.front();
    for (const Individual& ind : population) {
        if (ind.value > best.value || (ind.value == best.value && ind.weight < best.weight)) {
            best = ind;
        }
    }

    for (int generation = 0; generation < generations_; ++generation) {
        std::sort(population.begin(), population.end(), [](const Individual& lhs, const Individual& rhs) {
            if (lhs.value != rhs.value) {
                return lhs.value > rhs.value;
            }
            return lhs.weight < rhs.weight;
        });

        std::vector<Individual> next_generation;
        next_generation.reserve(population_size_);

        for (int i = 0; i < elite_count_ && i < static_cast<int>(population.size()); ++i) {
            next_generation.push_back(population[i]);
        }

        while (static_cast<int>(next_generation.size()) < population_size_) {
            const int first_parent = TournamentSelect(population);
            const int second_parent = TournamentSelect(population);

            Individual child = Crossover(population[first_parent], population[second_parent]);
            Mutate(child);
            next_generation.push_back(std::move(child));
        }

        population.swap(next_generation);

        for (const Individual& ind : population) {
            if (ind.value > best.value || (ind.value == best.value && ind.weight < best.weight)) {
                best = ind;
            }
        }
    }

    KnapsackSolution solution;
    solution.value = best.value;
    solution.weight = best.weight;
    solution.taken = best.taken;
    return solution;
}
