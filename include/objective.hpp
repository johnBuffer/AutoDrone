#pragma once
#include <cstdint>
#include <vector>


struct Objective
{
	uint32_t target_id;
	float time_in;
	float time_out;
	float points;

	Objective()
	{}

	void reset()
	{
		target_id = 0;
		time_in = 0.0f;
		time_out = 0.0f;
		points = 0.0f;
	}

	template<typename T>
	const T& getTarget(const std::vector<T>& targets) const
	{
		return targets[target_id];
	}

	template<typename T>
	void nextTarget(const std::vector<T>& targets)
	{
		target_id = (target_id + 1) % targets.size();
		time_in = 0.0f;
		time_out = 0.0f;
	}

	void addTimeIn(float dt)
	{
		time_in += dt;
	}

	void addTimeOut(float dt)
	{
		time_in = 0.0f;
		time_out += dt;
	}
};
