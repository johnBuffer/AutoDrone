#pragma once

#include "genom.hpp"


struct Unit
{
	Unit() = default;
	Unit(uint32_t genom)
		: genom_id(genom)
		, fitness(0.0f)
		, alive(true)
	{}

	virtual void setGenom(const nt::Genom& g) = 0;
	
	float fitness;
	bool alive;
	uint32_t genom_id;
};
