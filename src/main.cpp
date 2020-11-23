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
	return d.position.x > -tolerance * zone.x && d.position.x < (1.0f + tolerance)*zone.x && d.position.y > -tolerance * zone.y && d.position.y < (1.0f + tolerance) *zone.y;
}

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
	Graphic fitness_graph(200, sf::Vector2f(600, 100), sf::Vector2f(GUI_MARGIN, win_height - 100 - GUI_MARGIN));
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
	event_manager.addKeyPressedCallback(sf::Keyboard::Down, [&](sfev::CstEv ev) { drone.left.angle -= 0.1f; });*/

	const float score_coef = 10.0f;
	float best_score = 1.0f;
	while (window.isOpen()) {
		event_manager.processEvents();

		// Initialize drones
		std::vector<Drone>& population = stadium.getCurrentPopulation();
		for (Drone& d : population) {
			d.position = sf::Vector2f(win_width * 0.5f, win_height * 0.5f);
			d.reset();
		}

		const float border = 200.0f;
		sf::Vector2f target = sf::Vector2f(border + getRandUnder(win_width - 2.0f * border), border + getRandUnder(win_height - 2.0f * border));

		float time = 0.0f;
		float avg_fitness = 0.0f;

		while (getAlive(population) && window.isOpen() && (time < 100.0f || manual_control)) {
			event_manager.processEvents();

			if (manual_control) {
				const sf::Vector2i mouse_position = sf::Mouse::getPosition(window);
				target.x = mouse_position.x;
				target.y = mouse_position.y;
			}

			/*drone.left.power = boost_left * 20.0f;
			drone.right.power = boost_right * 20.0f;
			drone.update(dt);*/

			for (Drone& d : population) {
				if (d.alive) {
					const sf::Vector2f to_target = target - d.position;
					std::vector<float> inputs = { to_target.x / win_width, to_target.y / win_height, d.velocity.x * dt, d.velocity.y * dt, fmod(d.angle, 2.0f * PI) / (2.0f*PI), d.angular_velocity * dt };
					d.execute(inputs);
					d.update(dt);

					const float fitness_denom = std::max(1.0f, getLength(to_target));// *std::max(1.0f, std::abs(d.angular_velocity * dt));
					d.fitness += score_coef / fitness_denom;
					avg_fitness += d.fitness;
				}
			}

			for (Drone& d : population) {
				d.alive = checkAlive(d, sf::Vector2f(win_width, win_height), 0.1f);
			}

			fitness_graph.setLastValue(avg_fitness / float(pop_size));

			time += dt;

			// Render
			window.clear();

			if (draw_drones) {
				uint32_t drone_id = 0;
				for (Drone& d : population) {
					if (d.alive) {
						d.draw(window, colors[drone_id%colors.size()]);
						if (show_just_one) {
							break;
						}
					}
					++drone_id;
				}
			}
			
			const float target_r = 8.0f;
			sf::CircleShape target_c(target_r);
			target_c.setFillColor(sf::Color(255, 128, 0));
			target_c.setOrigin(target_r, target_r);
			target_c.setPosition(target);
			window.draw(target_c);

			// Print Network
			if (!full_speed && draw_neural) {
				for (Drone& d : population) {
					if (d.alive) {
						const sf::Vector2f to_target = target - d.position;
						std::vector<float> inputs = { to_target.x / win_width, to_target.y / win_height, d.velocity.x * dt, d.velocity.y * dt, d.getNormalizedAngle(), d.angular_velocity * dt };
						network_printer.render(window, d.network, inputs);
						break;
					}
				}
			}

			fitness_graph.render(window);

			//drone.draw(window);

			window.display();
		}
		
		if (!manual_control) {
			fitness_graph.next();
			stadium.nextGeneration();
		}
	}

	return 0;
}