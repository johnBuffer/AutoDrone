#pragma once

#include <vector>
#include <algorithm> // Include for std::min
#include "utils.hpp"
#include "number_generator.hpp"

struct SelectionWheel
{
    SelectionWheel(const uint64_t pop_size)
        : population_size(pop_size)
        , fitness_acc(pop_size + 1, 0.0f) // Initialize with 0.0f
        , current_index(0)
    {
        // fitness_acc[0] is already initialized to 0.0f
    }

    void reset()
    {
        current_index = 0;
    }

    void addFitnessScore(float score)
    {
        fitness_acc[current_index + 1] = fitness_acc[current_index] + score;
        ++current_index;
    }

    template<typename T>
    void addFitnessScores(const std::vector<T>& pop)
    {
        reset();
        const auto count = std::min(population_size, static_cast<uint64_t>(pop.size())); // Ensure same type for std::min
        for (uint64_t i = 0; i < count; ++i) {
            addFitnessScore(pop[i].fitness);
        }
    }

    float getAverageFitness() const
    {
        return fitness_acc.back() / static_cast<float>(population_size);
    }

    int64_t findClosestValueUnder(float f) const
    {
        int64_t b_inf = 0;
        int64_t b_sup = population_size;

        while (b_inf < b_sup - 1) {
            const int64_t index = (b_inf + b_sup) / 2;
            if (fitness_acc[index] < f) {
                b_inf = index;
            }
            else {
                b_sup = index;
            }
        }

        return (b_inf + b_sup) / 2;
    }

    int64_t pickTest(float value) const
    {
        int64_t result = population_size - 1;
        for (uint64_t i = 1; i < population_size + 1; ++i) {
            if (fitness_acc[i] > value) {
                result = i - 1;
                break;
            }
        }

        return result;
    }

    template<typename T>
    const T& pick(const std::vector<T>& population, uint64_t* index = nullptr) const
    {
        const float pick_value = NumberGenerator<>::getInstance().getUnder(fitness_acc.back());
        uint64_t picked_index = pickTest(pick_value);

        if (index) {
            *index = picked_index;
        }

        return population[picked_index];
    }

    const uint64_t population_size;
    std::vector<float> fitness_acc;
    uint64_t current_index;
};