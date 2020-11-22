#pragma once

#include "unit.hpp"
#include "neural_network.hpp"


struct AiUnit : public Unit
{
	AiUnit()
		: Unit(0)
	{}

	AiUnit(const std::vector<uint64_t>& network_architecture)
		: Unit(Network::getParametersCount(network_architecture) * 32)
		, network(network_architecture)
	{
		dna.initialize<float>(16.0f);
		updateNetwork();
	}

	void execute(const std::vector<float>& inputs)
	{
		process(network.execute(inputs));
	}

	void updateNetwork()
	{
		uint64_t index = 0;
		for (Layer& layer : network.layers) {
			const uint64_t neurons_count = layer.getNeuronsCount();
			for (uint64_t i(0); i < neurons_count; ++i) {
				layer.bias[i] = dna.get<float>(index++);
			}

			for (uint64_t i(0); i < neurons_count; ++i) {
				const uint64_t weights_count = layer.getWeightsCount();
				for (uint64_t j(0); j < weights_count; ++j) {
					layer.weights[i][j] = dna.get<float>(index++);
				}
			}
		}
	}

	void onUpdateDNA() override
	{
		updateNetwork();
	}

	virtual void process(const std::vector<float>& outputs) = 0;

	Network network;
};
