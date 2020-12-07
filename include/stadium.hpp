#pragma once

#include "selector.hpp"
#include "drone.hpp"
#include "objective.hpp"


struct Stadium
{
	struct TargetState
	{
		uint32_t id;
		float time_in;
		float time_out;
		float points;
	};

	struct Iteration
	{
		float time;
		float best_fitness;

		void reset()
		{
			time = 0.0f;
			best_fitness = 0.0f;
		}
	};

	uint32_t population_size;
	Selector<Drone> selector;
	uint32_t targets_count;
	std::vector<sf::Vector2f> targets;
	std::vector<TargetState> drones_state;
	sf::Vector2f area_size;
	Iteration current_iteration;
	bool use_manual_target;
	sf::Vector2f manual_target;

	Stadium(uint32_t population, sf::Vector2f size)
		: population_size(population)
		, selector(population)
		, targets_count(12)
		, targets(targets_count)
		, drones_state(population)
		, area_size(size)
		, use_manual_target(false)
	{
		
	}

	void initializeTargets()
	{
		// Initialize targets
		const float border = 150.0f;
		for (uint32_t i(0); i < targets_count; ++i) {
			targets[i] = sf::Vector2f(border + getRandUnder(area_size.x - 2.0f * border), border + getRandUnder(area_size.y - 2.0f * border));
		}
	}

	void initializeDrones()
	{
		// Initialize targets
		auto& drones = selector.getCurrentPopulation();
		for (Drone& d : drones) {
			d.position = area_size * 0.5f;
			d.reset();
		}
	}

	bool checkAlive(const Drone& drone, float tolerance) const
	{
		return std::abs(drone.angle) < PI;
	}

	uint32_t getAliveCount() const
	{
		uint32_t result = 0;
		const auto& drones = selector.getCurrentPopulation();
		for (const Drone& d : drones) {
			result += d.alive;
		}

		return result;
	}

	void updateDrone(uint32_t i, float dt, bool update_smoke)
	{
		const float target_radius = 16.0f;
		Drone& d = selector.getCurrentPopulation()[i];
		if (d.alive) {
			sf::Vector2f current_target = manual_target;

			sf::Vector2f standby_target;
			const float border = 50.0f;
			const float v_border = 120.0f;
			const uint32_t row_size = 7;
			const float width = (area_size.x - 2.0 * border) / float(row_size);
			const uint32_t effective_index = d.index;
			standby_target.x = border + (effective_index % row_size + 0.5f) * width;
			standby_target.y = v_border + (effective_index / row_size) * 150;

			if (!use_manual_target) {
				current_target = standby_target;
			}
			else if (i > 0) {
				const float angle = 2.0f * PI * (i / float(population_size));
				current_target = area_size * 0.5f + 1200.0f * sf::Vector2f(cos(angle), sin(angle));
			}
			
			const float max_dist = 700.0f;
			sf::Vector2f to_target = current_target - d.position;
			const float to_target_dist = getLength(to_target);
			to_target.x /= std::max(to_target_dist, max_dist);
			to_target.y /= std::max(to_target_dist, max_dist);

			const std::vector<float> inputs = {
				to_target.x,
				to_target.y,
				d.velocity.x * dt,
				d.velocity.y * dt,
				cos(d.angle),
				sin(d.angle),
				d.angular_velocity * dt
			};

			d.execute(inputs);
			d.update(dt, update_smoke);
			
			d.alive = checkAlive(d, 0.25f);
		}
	}

	void update(float dt, bool update_smoke)
	{
		uint64_t i(0);
		for (Drone& d : selector.getCurrentPopulation()) {
			updateDrone(i, dt, update_smoke);
			++i;
		}
	}

	void initializeIteration()
	{
		initializeTargets();
		initializeDrones();
		current_iteration.reset();
	}

	void nextIteration()
	{
		selector.nextGeneration();
	}
};
