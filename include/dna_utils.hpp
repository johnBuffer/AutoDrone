#pragma once
#include "dna.hpp"


struct DNAUtils
{
	static DNA crossover(const DNA& dna1, const DNA& dna2, const uint64_t cross_point)
	{
		const uint64_t code_size = dna1.code.size();
		DNA result(code_size * 8);

		for (uint64_t i(0); i < cross_point; ++i) {
			result.code[i] = dna1.code[i];
		}
		for (uint64_t i(cross_point); i < code_size; ++i) {
			result.code[i] = dna2.code[i];
		}

		return result;
	}

	template<typename T>
	static DNA makeChild(const DNA& dna1, const DNA& dna2, const float mutation_probability)
	{
		const uint64_t point1 = getIntUnderNonReset(as<uint32_t>(dna1.getBytesCount()));
		DNA child_dna = crossover(dna1, dna2, point1);
		const uint64_t element_count = dna1.getElementsCount<T>();
		for (uint64_t i(element_count - 1); i--;) {
			const float distrib = 1.0f + NumberGenerator<>::getInstance().get(mutation_probability);
			child_dna.set(i, child_dna.get<float>(i) * distrib);
		}
		child_dna.mutate<float>(mutation_probability);
		return child_dna;
	}

	template<typename T>
	static DNA evolve(const DNA& dna, float mutation_probability, float range)
	{
		DNA child_dna = dna;
		optimize<T>(child_dna, mutation_probability, range);
		return child_dna;
	}

	template<typename T>
	static void optimize(DNA& dna, const float probability, const float range)
	{
		const uint64_t element_count = dna.getElementsCount<T>();
		for (uint64_t i(element_count - 1); i--;) {
			if (pass(probability)) {
				const T value = dna.get<T>(i);
				const T random_offset = NumberGenerator<>::getInstance().get(range);
				dna.set(i, value + random_offset);
			}
		}
	}

	static bool pass(float probability)
	{
		return NumberGenerator<>::getInstance().getUnder(1.0f) < probability;
	}
};
