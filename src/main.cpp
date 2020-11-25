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
#include "stadium.hpp"


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

	const uint32_t pop_size = 800;
	Stadium stadium(pop_size, sf::Vector2f(win_width, win_height));

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
	const sf::Vector2f network_size = network_printer.getSize(3, 9);
	network_printer.position = sf::Vector2f(win_width - network_size.x - GUI_MARGIN, win_height - network_size.y - GUI_MARGIN);

	/*Drone drone(sf::Vector2f(win_width * 0.5f, win_height * 0.5f));
	bool boost_left = false;
	bool boost_right = false;
	event_manager.addKeyPressedCallback(sf::Keyboard::A, [&](sfev::CstEv ev) { boost_left = true; });
	event_manager.addKeyReleasedCallback(sf::Keyboard::A, [&](sfev::CstEv ev) { boost_left = false; });

	event_manager.addKeyPressedCallback(sf::Keyboard::E, [&](sfev::CstEv ev) { boost_right = true; });
	event_manager.addKeyReleasedCallback(sf::Keyboard::E, [&](sfev::CstEv ev) { boost_right = false; });

	event_manager.addKeyPressedCallback(sf::Keyboard::Up, [&](sfev::CstEv ev) { drone.left.target_angle += 0.1f; });
	event_manager.addKeyPressedCallback(sf::Keyboard::Down, [&](sfev::CstEv ev) { drone.left.target_angle -= 0.1f; });

	event_manager.addKeyPressedCallback(sf::Keyboard::Left, [&](sfev::CstEv ev) { drone.right.target_angle += 0.1f; });
	event_manager.addKeyPressedCallback(sf::Keyboard::Right, [&](sfev::CstEv ev) { drone.right.target_angle -= 0.1f; });*/

	DroneRenderer drone_renderer;

	sf::Clock clock;
	while (window.isOpen()) {
		event_manager.processEvents();

		// Initialize drones
		std::vector<Drone>& population = stadium.selector.getCurrentPopulation();
		stadium.initializeIteration();

		clock.restart();

		while (stadium.getAliveCount() && window.isOpen() && stadium.current_iteration.time < 100.0f) {
			event_manager.processEvents();

			if (manual_control) {
				const sf::Vector2i mouse_position = sf::Mouse::getPosition(window);
				mouse_target.x = mouse_position.x;
				mouse_target.y = mouse_position.y;
			}

			/*const float t_power = 1000.0f;
			drone.left.power = boost_left * t_power;
			drone.right.power = boost_right * t_power;
			drone.update(dt);*/

			stadium.update(dt);

			fitness_graph.setLastValue(stadium.current_iteration.best_fitness);

			// Render
			window.clear();

			uint32_t current_drone_i = 0;
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
				target_c.setPosition(stadium.targets[stadium.drones_state[current_drone_i].id]);
				window.draw(target_c);
			}

			// Print Network
			if (!full_speed && draw_neural) {
				for (Drone& d : population) {
					if (d.alive) {
						const sf::Vector2f to_target = stadium.targets[stadium.drones_state[current_drone_i].id] - d.position;
						std::vector<float> inputs = { normalize(to_target.x, win_width), normalize(to_target.y, win_height), d.velocity.x * dt, d.velocity.y * dt, cos(d.angle), sin(d.angle), d.angular_velocity * dt };
						network_printer.render(window, d.network, inputs);
						break;
					}
				}
			}

			fitness_graph.render(window);

			//drone_renderer.draw(drone, window, colors[0]);

			window.display();
		}
		
		//std::cout << "Iteration time: " << clock.getElapsedTime().asMilliseconds() << "ms" << std::endl;

		fitness_graph.next();
		stadium.nextIteration();
	}

	return 0;
}