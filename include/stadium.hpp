#pragma once

#include "selector.hpp"
#include "drone.hpp"


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
		const bool in_window = 
			drone.position.x > -tolerance * area_size.x
			&& drone.position.x < (1.0f + tolerance)*area_size.x
			&& drone.position.y > -tolerance * area_size.y
			&& drone.position.y < (1.0f + tolerance) * area_size.y;

		return in_window && std::abs(drone.angle) < PI;
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

			const sf::Vector2f standby_target = area_size * 0.5f;

			if (!use_manual_target) {
				current_target = standby_target;
			}
			
			const sf::Vector2f to_target = current_target - d.position;
			float to_x = normalize(to_target.x, area_size.x);
			float to_y = normalize(to_target.y, area_size.y);

			const float max_to_target = 0.4f;

			if (std::abs(to_x) > max_to_target) {
				to_x = sign(to_x) * max_to_target;
			}
			if (std::abs(to_y) > max_to_target) {
				to_y = sign(to_y) * max_to_target;
			}

			std::vector<float> inputs = {
				to_x,
				to_y,
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
