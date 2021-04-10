#pragma once

#include "unit.hpp"
#include "network.hpp"


struct AiUnit : public Unit
{
	AiUnit()
		: Unit(0)
		, network(NetworkInfo())
	{}

	void execute(const std::vector<float>& inputs)
	{
		process(network.execute(inputs));
	}

	void setGenom(const nt::Genom& g)
	{
		network = g.generateNetwork();
	}

	virtual void process(const std::vector<float>& outputs) = 0;

	nt::Network network;
};
