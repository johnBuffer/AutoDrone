#pragma once
#include "dna_utils.hpp"
#include "neural_network.hpp"
#include "game.hpp"
#include "selection_wheel.hpp"
#include "unit.hpp"
#include "double_buffer.hpp"


const float population_conservation_ratio = 0.1f;


template<typename T>
struct Selector
{
	Selector(const uint32_t agents_count)
		: population(agents_count)
		, population_size(agents_count)
		, current_iteration(0)
		, survivings_count(as<uint32_t>(agents_count * population_conservation_ratio))
		, wheel(survivings_count)
	{
	}

	void nextGeneration()
	{
		// Selection parameters
		// Create selection wheel
		sortCurrentPopulation();
		std::vector<T>& current_units = population.getCurrent();
		std::vector<T>& next_units    = population.getLast();
		wheel.addFitnessScores(current_units);
		// The top best survive;
		uint32_t evolve_count = 0;
		for (uint32_t i(0); i < survivings_count; ++i) {
			next_units[i] = current_units[i];
		}
		// Replace the weakest
		const float mutation_proba = 1.0f / std::max(1.0f, wheel.getAverageFitness());
		std::cout << "Avg fitness: " << wheel.getAverageFitness() << " mut prob: " << mutation_proba << std::endl;
		for (uint32_t i(survivings_count); i < population_size; ++i) {
			const T& unit_1 = wheel.pick(current_units);
			const T& unit_2 = wheel.pick(current_units);
			if (unit_1.dna == unit_2.dna) {
				++evolve_count;
				next_units[i].loadDNA(DNAUtils::evolve<float>(unit_1.dna, mutation_proba, 0.1f));
			}
			else {
				next_units[i].loadDNA(DNAUtils::makeChild<float>(unit_1.dna, unit_2.dna, mutation_proba));
			}
		}

		switchPopulation();
	}

	void sortCurrentPopulation()
	{
		std::vector<T>& current_units = population.getCurrent();
		std::sort(current_units.begin(), current_units.end(), [&](const T& a1, const T& a2) {return a1.fitness > a2.fitness; });
	}

	const T& getBest() const
	{
		return getNextPopulation()[0];
	}

	void switchPopulation()
	{
		population.swap();
		++current_iteration;
	}

	std::vector<T>& getCurrentPopulation()
	{
		return population.getCurrent();
	}

	std::vector<T>& getNextPopulation()
	{
		return population.getLast();
	}

	const std::vector<T>& getNextPopulation() const
	{
		return population.getLast();
	}

	const uint32_t population_size;
	const uint32_t survivings_count;
	DoubleObject<std::vector<T>> population;
	SelectionWheel wheel;

	uint32_t current_iteration;
};

