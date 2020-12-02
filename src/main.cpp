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
#include "dynamic_blur.hpp"


int main()
{
	NumberGenerator<>::initialize();

	const uint32_t win_width = 1600;
	const uint32_t win_height = 900;
	sf::ContextSettings settings;
	settings.antialiasingLevel = 4;
	sf::RenderWindow window(sf::VideoMode(win_width, win_height), "AutoDrone", sf::Style::Fullscreen, settings);
	window.setVerticalSyncEnabled(true);

	Blur blur(win_width, win_height, 1.0f);

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
	bool draw_neural = false;
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

	sf::RenderTexture blur_target;
	blur_target.create(win_width, win_height);

	const std::string dna_file_1 = "../selector_output_2.bin";
	const std::string dna_file_2 = "../selector_output_3.bin";
	const std::string dna_file_3 = "../selector_output_4.bin";
	const uint64_t dna_bytes_count = Network::getParametersCount(architecture) * 4;

	std::cout << "pop size 3 " << DnaLoader::getDnaCount(dna_file_3, dna_bytes_count) << std::endl;

	uint32_t pop_size = 35;
	Stadium stadium(pop_size, sf::Vector2f(win_width, win_height));
	event_manager.addKeyPressedCallback(sf::Keyboard::M, [&](sfev::CstEv) { stadium.use_manual_target = !stadium.use_manual_target; window.setMouseCursorVisible(!stadium.use_manual_target); });

	std::cout << "Pop size " << pop_size << std::endl;

	uint32_t current_drone = 0;

	sf::Font font;
	font.loadFromFile("../inconsolata.ttf");
	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(24);
	text.setFillColor(sf::Color::White);
	text.setPosition(2.0f * GUI_MARGIN, GUI_MARGIN);

	std::string logs = format(" Gen", 6) + char(124) + format(" Time (s)", 18) + "|" + format(" Collected", 10);
	logs += "\n" + format("", logs.size() + 1, '_');
	logs += "\n";

	// Initialize drones
	std::vector<Drone>& population = stadium.selector.getCurrentPopulation();
	stadium.initializeIteration();

	for (uint32_t i(0); i < 14; ++i) {
		const uint64_t gen = i;
		DNA dna = DnaLoader::loadDnaFrom(dna_file_1, dna_bytes_count, gen);
		stadium.selector.getCurrentPopulation()[i].loadDNA(dna);
		stadium.selector.getCurrentPopulation()[i].generation = std::max(uint64_t(1), 100 * gen);
		stadium.selector.getCurrentPopulation()[i].index = i;
	}

	for (uint32_t i(14); i < 28; ++i) {
		const uint64_t gen = i - 14;
		DNA dna = DnaLoader::loadDnaFrom(dna_file_2, dna_bytes_count, gen);
		stadium.selector.getCurrentPopulation()[i].loadDNA(dna);
		stadium.selector.getCurrentPopulation()[i].generation = 2000 + 100 * gen;
		stadium.selector.getCurrentPopulation()[i].index = i;
	}

	for (uint32_t i(28); i < 35; ++i) {
		const uint64_t gen = i - 28 + 20;
		DNA dna = DnaLoader::loadDnaFrom(dna_file_3, dna_bytes_count, gen);
		stadium.selector.getCurrentPopulation()[i].loadDNA(dna);
		stadium.selector.getCurrentPopulation()[i].generation = 3000 + 100 * gen;
		stadium.selector.getCurrentPopulation()[i].index = i;
	}

	if (stadium.use_manual_target) {
		const sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);
		stadium.manual_target.x = mouse_pos.x;
		stadium.manual_target.y = mouse_pos.y;
	}

	std::vector<int32_t> drones_id{0, 1, 2, 5, 10, 20, 25, 33};

	sf::Clock clock;
	while (window.isOpen()) {
		event_manager.processEvents();

		stadium.current_iteration.time = 0.0f;

		const uint32_t drone_id = drones_id[current_drone % drones_id.size()];
		Drone& drone = stadium.selector.getCurrentPopulation()[drone_id];

		text.setString(logs);

		while (drone.alive &&
			!drone.done &&
			window.isOpen() &&
			stadium.current_iteration.time < 100.0f ||
			current_drone == drones_id.size())
		{
			event_manager.processEvents();
			
			stadium.current_iteration.time += dt;

			if (stadium.use_manual_target) {
				const sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);
				stadium.manual_target.x = mouse_pos.x;
				stadium.manual_target.y = mouse_pos.y;
			}

			if (clock.getElapsedTime().asSeconds() > 5.0f) {
				stadium.updateDrone(drone_id, dt, true);
			}

			// Render
			window.clear();
			blur_target.clear();

			window.draw(text);

			if (current_drone < drones_id.size() && clock.getElapsedTime().asSeconds() > 5.0f) {
				drone_renderer.draw(drone, window, blur_target, state, colors[drone_id%colors.size()], !full_speed);

				sf::CircleShape target_c(target_radius);
				target_c.setFillColor(sf::Color(255, 128, 0));
				target_c.setOrigin(target_radius, target_radius);
				target_c.setPosition(stadium.targets[stadium.drones_state[drone_id].id]);
				window.draw(target_c);
				target_c.setFillColor(sf::Color::Black);
				blur_target.draw(target_c);

				// Print Network

				if (draw_neural) {
					network_printer.render(window, drone.network);
				}
			}

			if (draw_fitness) {
				fitness_graph.render(window);
			}

			blur_target.display();
			sf::Sprite bloom_sprite = blur.apply(blur_target.getTexture(), 2);

			window.draw(bloom_sprite, sf::BlendAdd);

			window.display();
		}

		std::string dead = "[COMPLETE]";
		if (!drone.alive) {
			dead = "[CRASH]";
		}
		else if (stadium.current_iteration.time >= 100.0f) {
			dead = "[TIME OUT]";
		}

		logs += "\n" + format(" " + toString(drone.generation), 6) + "|" + format(" " + toString(drone.total_time, 1), 6) + format(" " + dead, 12) + "|" + format(" " + toString(drone.collected), 3, ' ', true) + " / " + toString(stadium.targets_count);

		++current_drone;
	}

	return 0;
}