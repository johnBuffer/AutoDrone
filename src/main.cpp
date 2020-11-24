#include <SFML/Graphics.hpp>
#include <vector>
#include <list>
#include <event_manager.hpp>
#include <iostream>

#include "game.hpp"
#include "dna.hpp"
#include "selector.hpp"
#include "number_generator.hpp"
#include "neural_renderer.hpp"
#include "graph.hpp"
#include "drone.hpp"
#include "drone_renderer.hpp"


uint32_t getAlive(const std::vector<Drone>& drones)
{
	uint32_t result = 0;
	for (const Drone& d : drones) {
		result += d.alive;
	}

	return result;
}

bool checkAlive(const Drone& d, sf::Vector2f zone, float tolerance)
{
	const bool in_window = d.position.x > -tolerance * zone.x && d.position.x < (1.0f + tolerance)*zone.x && d.position.y > -tolerance * zone.y && d.position.y < (1.0f + tolerance) *zone.y;
	return in_window && std::abs(d.angle) < PI;
}

struct Target
{
	uint32_t id;
	float time_in;
	float time_out;
};

int main()
{
	NumberGenerator<>::initialize();

	const uint32_t win_width = 1600;
	const uint32_t win_height = 900;
	sf::ContextSettings settings;
	settings.antialiasingLevel = 4;
	sf::RenderWindow window(sf::VideoMode(win_width, win_height), "AutoDrone", sf::Style::Default, settings);
	window.setVerticalSyncEnabled(true);

	bool slow_motion = false;
	const float base_dt = 0.008f;
	float dt = base_dt;

	sfev::EventManager event_manager(window);
	event_manager.addEventCallback(sf::Event::Closed, [&](sfev::CstEv ev) { window.close(); });
	event_manager.addKeyPressedCallback(sf::Keyboard::Escape, [&](sfev::CstEv ev) { window.close(); });

	const uint32_t pop_size = 200;
	Selector<Drone> stadium(pop_size);
	std::vector<sf::Color> colors({ sf::Color(80, 81, 79), 
		                            sf::Color(121, 85, 83),
									sf::Color(161, 88, 86),
									sf::Color(242, 95, 92),
									sf::Color(249, 160, 97),
									sf::Color(255, 224, 102),
									sf::Color(146, 174, 131),
									sf::Color(36, 123, 160),
									sf::Color(74, 158, 170),
									sf::Color(112, 193, 179) });

	sf::Vector2f mouse_target;
	const float target_radius = 8.0f;
	const uint32_t targets_count = 10;
	std::vector<sf::Vector2f> targets(targets_count);
	std::vector<Target> drones_target(pop_size);

	const float border = 200.0f;
	targets[0] = sf::Vector2f(border + getRandUnder(win_width - 2.0f * border), border + getRandUnder(win_height - 2.0f * border));

	bool show_just_one = false;
	bool full_speed = false;
	bool manual_control = false;
	bool draw_neural = true;
	bool draw_drones = true;
	event_manager.addKeyPressedCallback(sf::Keyboard::E, [&](sfev::CstEv ev) { full_speed = !full_speed; window.setVerticalSyncEnabled(!full_speed); });
	event_manager.addKeyPressedCallback(sf::Keyboard::M, [&](sfev::CstEv ev) { manual_control = !manual_control; });
	event_manager.addKeyPressedCallback(sf::Keyboard::S, [&](sfev::CstEv ev) { show_just_one = !show_just_one; });
	event_manager.addKeyPressedCallback(sf::Keyboard::N, [&](sfev::CstEv ev) { draw_neural = !draw_neural; });
	event_manager.addKeyPressedCallback(sf::Keyboard::D, [&](sfev::CstEv ev) { draw_drones = !draw_drones; });

	const float GUI_MARGIN = 10.0f;
	Graphic fitness_graph(1000, sf::Vector2f(win_width - 2.0f * GUI_MARGIN, 100), sf::Vector2f(GUI_MARGIN, win_height - 100 - GUI_MARGIN));
	fitness_graph.color = sf::Color(96, 211, 148);
	Graphic bestGraph(200, sf::Vector2f(600, 100), sf::Vector2f(GUI_MARGIN, win_height - 200 - 2.0f * GUI_MARGIN));
	bestGraph.color = sf::Color(238, 96, 85);

	NeuralRenderer network_printer;
	const sf::Vector2f network_size = network_printer.getSize(4, 9);
	network_printer.position = sf::Vector2f(win_width - network_size.x - GUI_MARGIN, win_height - network_size.y - GUI_MARGIN);

	/*Drone drone(sf::Vector2f(win_width * 0.5f, win_height * 0.5f));
	bool boost_left = false;
	bool boost_right = false;
	event_manager.addKeyPressedCallback(sf::Keyboard::A, [&](sfev::CstEv ev) { boost_left = true; });
	event_manager.addKeyReleasedCallback(sf::Keyboard::A, [&](sfev::CstEv ev) { boost_left = false; });

	event_manager.addKeyPressedCallback(sf::Keyboard::E, [&](sfev::CstEv ev) { boost_right = true; });
	event_manager.addKeyReleasedCallback(sf::Keyboard::E, [&](sfev::CstEv ev) { boost_right = false; });

	event_manager.addKeyPressedCallback(sf::Keyboard::Up, [&](sfev::CstEv ev) { drone.left.angle += 0.1f; });
	event_manager.addKeyPressedCallback(sf::Keyboard::Down, [&](sfev::CstEv ev) { drone.left.angle -= 0.1f; });

	event_manager.addKeyPressedCallback(sf::Keyboard::Left, [&](sfev::CstEv ev) { drone.right.angle += 0.1f; });
	event_manager.addKeyPressedCallback(sf::Keyboard::Right, [&](sfev::CstEv ev) { drone.right.angle -= 0.1f; });*/

	DroneRenderer drone_renderer;

	const float score_coef = 1.0f;
	float best_score = 1.0f;
	while (window.isOpen()) {
		event_manager.processEvents();

		// Initialize drones
		std::vector<Drone>& population = stadium.getCurrentPopulation();
		uint32_t current_drone_i = 0;
		for (Drone& d : population) {
			drones_target[current_drone_i].id = 0;
			drones_target[current_drone_i].time_in = 0.0f;
			drones_target[current_drone_i].time_out = 0.0f;
			d.position = sf::Vector2f(win_width * 0.5f, win_height * 0.5f);
			d.reset();
			++current_drone_i;
		}

		// Initialize targets
		for (uint32_t i(0); i < targets_count; ++i) {
			const float border = 200.0f;
			targets[i] = sf::Vector2f(border + getRandUnder(win_width - 2.0f * border), border + getRandUnder(win_height - 2.0f * border));
		}

		float time = 0.0f;
		float avg_fitness = 0.0f;

		while (getAlive(population) && window.isOpen() && time < 100.0f) {
			event_manager.processEvents();

			if (manual_control) {
				const sf::Vector2i mouse_position = sf::Mouse::getPosition(window);
				mouse_target.x = mouse_position.x;
				mouse_target.y = mouse_position.y;
			}

			/*drone.left.power = boost_left * 400.0f;
			drone.right.power = boost_right * 400.0f;
			drone.update(dt);*/

			current_drone_i = 0;
			for (Drone& d : population) {
				if (d.alive) {
					const sf::Vector2f to_target = (manual_control ? mouse_target : targets[drones_target[current_drone_i].id]) - d.position;
					
					const float amp_factor = 4.0f;
					float to_x = sign(to_target.x) * std::min(1.0f, amp_factor * std::abs(normalize(to_target.x, win_width)));
					float to_y = sign(to_target.y) * std::min(1.0f, amp_factor * std::abs(normalize(to_target.y, win_height)));

					std::vector<float> inputs = { to_x, to_y, d.velocity.x * dt, d.velocity.y * dt, cos(d.angle), sin(d.angle), d.angular_velocity * dt };
					d.execute(inputs);
					d.update(dt);

					const float to_target_dist = getLength(to_target);
					const float fitness_denom = std::max(1.0f, to_target_dist) * (1.0f + std::abs(d.angle));
					d.fitness += score_coef / fitness_denom;
					avg_fitness += d.fitness;

					// Next target if needed
					const float target_reward = 1000.0f;
					const float target_time = 3.0f;
					if (to_target_dist < target_radius + d.radius) {
						drones_target[current_drone_i].time_in += dt;
						if (drones_target[current_drone_i].time_in > target_time) {
							d.fitness += target_reward / (1.0f + drones_target[current_drone_i].time_out);
							drones_target[current_drone_i].time_out = 0.0f;
							drones_target[current_drone_i].id = (drones_target[current_drone_i].id + 1) % targets_count;
						}
					}
					else {
						drones_target[current_drone_i].time_out += dt;
						drones_target[current_drone_i].time_in = 0.0f;
					}
				}
				++current_drone_i;
			}

			for (Drone& d : population) {
				d.alive = checkAlive(d, sf::Vector2f(win_width, win_height), 0.1f);
			}

			fitness_graph.setLastValue(avg_fitness / float(pop_size));

			time += dt;

			// Render
			window.clear();

			current_drone_i = 0;
			if (draw_drones) {
				for (Drone& d : population) {
					if (d.alive) {
						drone_renderer.draw(d, window, colors[current_drone_i%colors.size()]);
						if (show_just_one) {
							break;
						}
					}
					++current_drone_i;
				}
			}
			
			if (show_just_one) {
				sf::CircleShape target_c(target_radius);
				target_c.setFillColor(sf::Color(255, 128, 0));
				target_c.setOrigin(target_radius, target_radius);
				target_c.setPosition(manual_control ? mouse_target : targets[drones_target[current_drone_i].id]);
				window.draw(target_c);
			}

			// Print Network
			if (!full_speed && draw_neural) {
				for (Drone& d : population) {
					if (d.alive) {
						const sf::Vector2f to_target = manual_control ? mouse_target : targets[drones_target[current_drone_i].id] - d.position;
						std::vector<float> inputs = { normalize(to_target.x, win_width), normalize(to_target.y, win_height), d.velocity.x * dt, d.velocity.y * dt, cos(d.angle), sin(d.angle), d.angular_velocity * dt };
						network_printer.render(window, d.network, inputs);
						break;
					}
				}
			}

			fitness_graph.render(window);

			//drone_renderer.draw(drone, window);

			window.display();
		}
		
		if (!manual_control) {
			fitness_graph.next();
			stadium.nextGeneration();
		}
	}

	return 0;
}