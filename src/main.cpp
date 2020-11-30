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
#include "dna_loader.hpp"


int main()
{
	NumberGenerator<>::initialize();

	const uint32_t win_width = 1600;
	const uint32_t win_height = 800;
	sf::ContextSettings settings;
	settings.antialiasingLevel = 4;
	sf::RenderWindow window(sf::VideoMode(win_width, win_height), "AutoDrone", sf::Style::Default, settings);
	window.setVerticalSyncEnabled(true);

	bool slow_motion = false;
	const float base_dt = 0.007f;
	float dt = base_dt;

	sfev::EventManager event_manager(window);
	event_manager.addEventCallback(sf::Event::Closed, [&](sfev::CstEv ev) { window.close(); });
	event_manager.addKeyPressedCallback(sf::Keyboard::Escape, [&](sfev::CstEv ev) { window.close(); });

	std::vector<sf::Color> colors({ sf::Color(36, 123, 160),
									sf::Color(161, 88, 86),
									sf::Color(249, 160, 97),
									sf::Color(80, 81, 79), 
		                            sf::Color(121, 85, 83),
									sf::Color(242, 95, 92),
									sf::Color(255, 224, 102),
									sf::Color(146, 174, 131),
									sf::Color(74, 158, 170),
									sf::Color(112, 193, 179) });

	sf::Vector2f mouse_target;
	const float target_radius = 8.0f;

	bool show_just_one = false;
	bool full_speed = false;
	bool draw_neural = true;
	bool draw_drones = true;
	bool draw_fitness = true;

	event_manager.addKeyPressedCallback(sf::Keyboard::E, [&](sfev::CstEv ev) { full_speed = !full_speed; window.setVerticalSyncEnabled(!full_speed); });
	event_manager.addKeyPressedCallback(sf::Keyboard::S, [&](sfev::CstEv ev) { show_just_one = !show_just_one; });
	event_manager.addKeyPressedCallback(sf::Keyboard::N, [&](sfev::CstEv ev) { draw_neural = !draw_neural; });
	event_manager.addKeyPressedCallback(sf::Keyboard::D, [&](sfev::CstEv ev) { draw_drones = !draw_drones; });
	event_manager.addKeyPressedCallback(sf::Keyboard::F, [&](sfev::CstEv ev) { draw_fitness = !draw_fitness; });

	const float GUI_MARGIN = 10.0f;
	Graphic fitness_graph(1000, sf::Vector2f(700, 120), sf::Vector2f(GUI_MARGIN, win_height - 120 - GUI_MARGIN));
	fitness_graph.color = sf::Color(96, 211, 148);
	
	NeuralRenderer network_printer;
	const sf::Vector2f network_size = network_printer.getSize(4, 9);
	network_printer.position = sf::Vector2f(win_width - network_size.x - GUI_MARGIN, win_height - network_size.y - GUI_MARGIN);

	DroneRenderer drone_renderer;
	sf::RenderStates state;

	const uint32_t pop_size = 25;
	Stadium stadium(pop_size, sf::Vector2f(win_width, win_height));
	event_manager.addKeyPressedCallback(sf::Keyboard::M, [&](sfev::CstEv) { stadium.use_manual_target = !stadium.use_manual_target; });

	sf::Clock clock;
	while (window.isOpen()) {
		event_manager.processEvents();

		// Initialize drones
		std::vector<Drone>& population = stadium.selector.getCurrentPopulation();
		stadium.initializeIteration();

		for (uint32_t i(0); i < pop_size; ++i) {
			DNA dna = DnaLoader::loadDnaFrom("../selector_output_2.bin", Network::getParametersCount(architecture) * 4, i);
			stadium.selector.getCurrentPopulation()[i].loadDNA(dna);
			stadium.selector.getCurrentPopulation()[i].generation = 100 * i + 1;
		}
		
		if (stadium.use_manual_target) {
			const sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);
			stadium.manual_target.x = mouse_pos.x;
			stadium.manual_target.y = mouse_pos.y;
		}

		clock.restart();

		while (stadium.getAliveCount() && window.isOpen() && stadium.current_iteration.time < 100.0f) {
			event_manager.processEvents();

			if (stadium.use_manual_target) {
				const sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);
				stadium.manual_target.x = mouse_pos.x;
				stadium.manual_target.y = mouse_pos.y;
			}

			stadium.update(dt, !full_speed);

			// Render
			window.clear(/*sf::Color(191, 219, 247)*/);

			uint32_t current_drone_i = 0;
			if (draw_drones) {
				for (Drone& d : population) {
					if (d.alive) {
						drone_renderer.draw(d, window, state, colors[current_drone_i%colors.size()], !full_speed);
						if (show_just_one) {
							break;
						}
					}
					++current_drone_i;
				}
			}
			
			sf::CircleShape target_c(target_radius);
			target_c.setFillColor(sf::Color(255, 128, 0));
			target_c.setOrigin(target_radius, target_radius);
			target_c.setPosition(stadium.use_manual_target ? stadium.manual_target : stadium.targets[stadium.drones_state[current_drone_i].id]);
			window.draw(target_c);

			// Print Network
			if (!full_speed && draw_neural) {
				for (Drone& d : population) {
					if (d.alive) {
						network_printer.render(window, d.network);
						break;
					}
				}
			}

			if (draw_fitness) {
				fitness_graph.render(window);
			}

			window.display();
		}
		
		fitness_graph.next();
	}

	return 0;
}