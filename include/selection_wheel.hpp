#pragma once

#include <vector>
#include "utils.hpp"
#include "number_generator.hpp"


struct SelectionWheel
{
	SelectionWheel(const uint64_t pop_size)
		: population_size(pop_size)
		, fitness_acc(pop_size + 1)
		, current_index(0)
	{
		fitness_acc[0] = 0.0f;
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
	void addFitnessScores(std::vector<T>& pop)
	{
		reset();
		const uint64_t count = std::min(population_size, pop.size());
		for (uint64_t i(0); i < count; ++i) {
			addFitnessScore(pop[i].fitness);
		}
	}

	float getAverageFitness() const
	{
		return fitness_acc.back() / float(population_size);
	}

	int64_t findClosestValueUnder(float f) const
	{
		int64_t b_inf = 0;
		int64_t b_sup = population_size;

		do {
			const int64_t index = (b_inf + b_sup) >> 1;
			if (fitness_acc[index] < f) {
				b_inf = index;
			}
			else {
				b_sup = index;
			}
		} while (b_inf < b_sup - 1);

		return (b_inf + b_sup) >> 1;
	}

	int64_t pickTest(float value)
	{
		int64_t result = population_size - 1;
		for (uint64_t i(1); i < population_size + 1; ++i) {
			if (fitness_acc[i] > value) {
				result = i - 1;
				break;
			}
		}

		return result;
	}

	template<typename T>
	const T& pick(const std::vector<T>& population, uint64_t* index = nullptr)
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

