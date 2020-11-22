#pragma once

#include <vector>
#include "utils.hpp"


struct Layer
{
	Layer(const uint64_t neurons_count, const uint64_t prev_count)
		: weights(neurons_count)
		, values(neurons_count)
		, bias(neurons_count)
	{
		for (std::vector<float>& v : weights) {
			v.resize(prev_count);
		}
	}

	uint64_t getNeuronsCount() const
	{
		return bias.size();
	}

	uint64_t getWeightsCount() const
	{
		return weights.front().size();
	}

	void process(const std::vector<float>& inputs)
	{
		const uint64_t neurons_count = bias.size();
		const uint64_t inputs_count = inputs.size();
		// For each neuron
		for (uint64_t i(0); i < neurons_count; ++i) {
			float result = -bias[i];
			// Compute weighted sum of inputs
			for (uint64_t j(0); j < inputs_count; ++j) {
				result += weights[i][j] * inputs[j];
			}
			// Output result
			values[i] = 0.5f * (1.0f + sigm(result));
		}
	}

	void print() const
	{
		std::cout << "--- layer ---" << std::endl;
		const uint64_t neurons_count = values.size();
		for (uint64_t i(0); i < neurons_count; ++i) {
			const uint64_t inputs_count = getWeightsCount();
			// Compute weighted sum of inputs
			std::cout << "Neuron " << i << " bias " << bias[i] << std::endl;
			for (uint64_t j(0); j < inputs_count; ++j) {
				std::cout << weights[i][j] << " ";
			}
			std::cout << std::endl;
		}
		std::cout << "--- end ---\n" << std::endl;
	}

	std::vector<std::vector<float>> weights;
	std::vector<float> values;
	std::vector<float> bias;
};


struct Network
{
	Network()
		: input_size(0)
	{}

	Network(const uint64_t input_size_)
		: input_size(input_size_)
	{}

	Network(const std::vector<uint64_t>& layers_sizes)
		: input_size(layers_sizes[0])
	{
		for (uint64_t i(1); i < layers_sizes.size(); ++i) {
			addLayer(layers_sizes[i]);
		}
	}

	void addLayer(const uint64_t neurons_count)
	{
		if (!layers.empty()) {
			layers.emplace_back(neurons_count, layers.back().getNeuronsCount());
		}
		else {
			layers.emplace_back(neurons_count, input_size);
		}
	}

	const std::vector<float>& execute(const std::vector<float>& input)
	{
		if (input.size() == input_size) {
			layers.front().process(input);
			const uint64_t layers_count = layers.size();
			for (uint64_t i(1); i < layers_count; ++i) {
				layers[i].process(layers[i - 1].values);
			}
		}

		return layers.back().values;
	}

	uint64_t getParametersCount() const
	{
		uint64_t result = 0;
		for (const Layer& layer : layers) {
			result += layer.bias.size() * (1 + layer.weights.front().size());
		}
		return result;
	}

	static uint64_t getParametersCount(const std::vector<uint64_t>& layers_sizes)
	{
		uint64_t count = 0;
		for (uint64_t i(1); i < layers_sizes.size(); ++i) {
			count += layers_sizes[i] * (1 + layers_sizes[i - 1]);
		}
		return count;
	}

	uint64_t input_size;
	std::vector<Layer> layers;
};
