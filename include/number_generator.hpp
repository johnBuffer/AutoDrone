#pragma once

#include <random>
#include <limits>
#include <memory>


template<typename T = void>
struct NumberGenerator
{
	NumberGenerator(bool random_seed = true)
		: distribution(-1.0f, 1.0f)
	{
		if (random_seed) {
			gen = std::mt19937(rd());
		}
		else {
			gen = std::mt19937(0);
		}
	}

	float get(float range = 1.0f)
	{
		return range * distribution(gen);
	}

	float getUnder(float max_value)
	{
		return (distribution(gen) + 1.0f) * 0.5f * max_value;
	}

	float getMaxRange()
	{
		return std::numeric_limits<float>::max() * distribution(gen);
	}

	void reset()
	{
		// TODO add random seed case
		gen = std::mt19937(0);
		distribution.reset();
	}

	static NumberGenerator& getInstance()
	{
		return *s_instance;
	}

	static void initialize()
	{
		s_instance = std::make_unique<NumberGenerator>(false);
	}

	std::uniform_real_distribution<float> distribution;
	std::random_device rd;
	std::mt19937 gen;

	static std::unique_ptr<NumberGenerator> s_instance;
};


template<typename T>
std::unique_ptr<NumberGenerator<T>> NumberGenerator<T>::s_instance;

