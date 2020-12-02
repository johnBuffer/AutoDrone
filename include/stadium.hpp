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
		, targets_count(10)
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
		for (uint32_t i(100); i--;) {
			getRandUnder(1.0f);
		}
		for (uint32_t i(0); i < targets_count; ++i) {
			targets[i] = sf::Vector2f(border + getRandUnder(area_size.x - 2.0f * border), border + getRandUnder(area_size.y - 2.0f * border));
		}
	}

	void initializeDrones()
	{
		// Initialize targets
		auto& drones = selector.getCurrentPopulation();
		uint32_t current_drone_i = 0;
		for (Drone& d : drones) {
			TargetState& state = drones_state[current_drone_i];
			state.id = 0;
			state.time_in = 0.0f;
			state.time_out = 0.0f;
			d.position = sf::Vector2f(area_size.x * 0.5f, area_size.y * 0.5f);
			state.points = getLength(d.position - targets[0]);
			d.reset();
			++current_drone_i;
		}
	}

	bool checkAlive(const Drone& drone, float tolerance) const
	{
		const bool in_window = 
			drone.position.x > -tolerance * area_size.x
			&& drone.position.x < (1.0f + tolerance)*area_size.x
			&& drone.position.y > -tolerance * area_size.y
			&& drone.position.y < (1.0f + tolerance) * area_size.y;

		return in_window/* && std::abs(drone.angle) < PI*/;
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
		const float target_radius = 8.0f;
		Drone& d = selector.getCurrentPopulation()[i];
		if (d.alive) {
			sf::Vector2f current_target = use_manual_target ? manual_target : targets[drones_state[i].id];
			if (d.collected == targets_count && !use_manual_target) {
				const float border = 50.0f;
				const float v_border = 120.0f;
				const uint32_t row_size = 7;
				const float width = (area_size.x - 2.0 * border) / float(row_size);
				current_target.x = border + (d.index % row_size + 0.5f) * width;
				current_target.y = v_border + (d.index / row_size) * 150;
			}
			const sf::Vector2f to_target = current_target - d.position;

			std::vector<float> inputs = {
				normalize(to_target.x, area_size.x),
				normalize(to_target.y, area_size.y),
				d.velocity.x * dt,
				d.velocity.y * dt,
				cos(d.angle),
				sin(d.angle),
				d.angular_velocity * dt
			};

			if (d.collected < targets_count) {
				d.total_time += dt;
			}

			d.execute(inputs);
			d.update(dt, update_smoke);
			
			// We don't want weirdos
			const float to_target_dist = getLength(to_target);

			d.alive = checkAlive(d, 0.5f);

			TargetState& state = drones_state[i];

			d.done = d.collected == targets_count;

			// Next target if needed
			const float target_time = 3.0f;
			if (to_target_dist < target_radius + d.radius && !use_manual_target) {
				state.time_in += dt;
				if (state.time_in > target_time) {
					if (d.collected < targets_count) {
						state.time_out = 0.0f;
						state.id = (state.id + 1) % targets_count;
						state.points = getLength(d.position - targets[state.id]);
						++d.collected;
					}
				}
			}
			else {
				state.time_out += dt;
				state.time_in = 0.0f;
			}
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
