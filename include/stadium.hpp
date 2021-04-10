#pragma once

#include "neat.hpp"
#include "drone.hpp"
#include "objective.hpp"


struct Stadium
{
	struct Iteration
	{
		float time;
		float best_fitness;
		uint32_t best_unit;

		void reset()
		{
			time = 0.0f;
			best_fitness = 0.0f;
			best_unit = 0;
		}
	};

	uint32_t population_size;
	nt::Selector selector;
	uint32_t targets_count;
	std::vector<sf::Vector2f> targets;
	std::vector<Objective> objectives;
	sf::Vector2f area_size;
	Iteration current_iteration;
	std::vector<Drone> drones;
	float max_iteration_time;
	uint32_t generation;

	Stadium(uint32_t population, sf::Vector2f size)
		: population_size(population)
		, selector(population)
		, targets_count(10)
		, targets(targets_count)
		, objectives(population)
		, area_size(size)
		, max_iteration_time(100.0f)
		, generation(0)
		, drones(population)
	{
		for (nt::Genom& g : selector.population) {
			g = nt::Genom(7, 4);
		}
	}

	void initializeTargets()
	{
		// Initialize targets
		const float border = 200.0f;
		for (uint32_t i(0); i < targets_count; ++i) {
			targets[i] = sf::Vector2f(border + getRandUnder(area_size.x - 2.0f * border), border + getRandUnder(area_size.y - 2.0f * border));
		}
	}

	void initializeDrones()
	{
		// Initialize targets
		uint32_t i = 0;
		for (Drone& d : drones) {
			d.index = i++;
			Objective& objective = objectives[d.index];
			d.position = 0.5f * area_size;
			objective.reset();
			objective.points = getLength(d.position - targets[0]);
			d.reset();
			d.setGenom(selector.population[d.index]);
		}
	}

	bool checkAlive(const Drone& drone, float tolerance) const
	{
		const sf::Vector2f tolerance_margin = sf::Vector2f(tolerance, tolerance);
		const bool in_window = sf::FloatRect(-tolerance_margin, area_size + tolerance_margin).contains(drone.position);
		return in_window && std::abs(drone.angle) < PI;
	}

	uint32_t getAliveCount() const
	{
		uint32_t result = 0;
		for (const Drone& d : drones) {
			result += d.alive;
		}

		return result;
	}

	void updateDrone(uint64_t i, float dt, bool update_smoke)
	{
		Drone& d = drones[i];
		if (!d.alive) {
			// It's too late for it
			return;
		}

		const float target_radius = 8.0f;
		const float max_dist = 700.0f;
		const float tolerance_margin = 50.0f;
		
		Objective& objective = objectives[d.index];
		sf::Vector2f to_target = objective.getTarget(targets) - d.position;
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

		// The actual update
		d.execute(inputs);
		d.update(dt, update_smoke);
		d.alive = checkAlive(d, tolerance_margin);

		// Fitness stuffs
		d.fitness += 1.0f / (1.0f + to_target_dist);
		// We don't want weirdos
		const float score_factor = std::pow(cos(d.angle), 2.0f);
		const float target_time = 1.0f;
		if (to_target_dist < target_radius + d.radius) {
			objective.addTimeIn(dt);
			if (objective.time_in > target_time) {
				d.fitness += score_factor * objective.points / (1.0f + objective.time_out);
				objective.nextTarget(targets);
				objective.points = getLength(d.position - objective.getTarget(targets));
			}
		}
		else {
			objective.addTimeOut(dt);
		}

		checkBestFitness(d.fitness, d.index);
	}

	void checkBestFitness(float fitness, uint32_t id)
	{
		if (fitness > current_iteration.best_fitness) {
			current_iteration.best_fitness = fitness;
			current_iteration.best_unit = id;
		}
	}

	void update(float dt, bool update_smoke)
	{
		for (uint32_t i(0); i<population_size; ++i) {
			updateDrone(i, dt, update_smoke);
		}
		current_iteration.time += dt;
	}

	void newIteration()
	{
		if (generation) {
			for (const Drone& d : drones) {
				selector.population[d.index].fitness = d.fitness;
			}
			selector.nextGeneration();
		}
		initializeTargets();
		initializeDrones();
		current_iteration.reset();
		++generation;
	}

	bool isDone() const
	{
		return getAliveCount() == 0 || current_iteration.time > max_iteration_time || generation == 0;
	}
};
