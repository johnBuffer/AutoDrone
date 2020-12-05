#include <SFML/Graphics.hpp>
#include <event_manager.hpp>
#include <iostream>

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

	const uint32_t win_width = 1920;
	const uint32_t win_height = 1080;
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

	// The drones colors
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
	bool manual_control = false;
	bool draw_neural = true;
	bool draw_drones = true;
	bool draw_fitness = true;
	event_manager.addKeyPressedCallback(sf::Keyboard::E, [&](sfev::CstEv ev) { full_speed = !full_speed; window.setVerticalSyncEnabled(!full_speed); });
	event_manager.addKeyPressedCallback(sf::Keyboard::M, [&](sfev::CstEv ev) { manual_control = !manual_control; });
	event_manager.addKeyPressedCallback(sf::Keyboard::S, [&](sfev::CstEv ev) { show_just_one = !show_just_one; });
	event_manager.addKeyPressedCallback(sf::Keyboard::N, [&](sfev::CstEv ev) { draw_neural = !draw_neural; });
	event_manager.addKeyPressedCallback(sf::Keyboard::D, [&](sfev::CstEv ev) { draw_drones = !draw_drones; });
	event_manager.addKeyPressedCallback(sf::Keyboard::F, [&](sfev::CstEv ev) { draw_fitness = !draw_fitness; });

	const float GUI_MARGIN = 10.0f;
	Graphic fitness_graph(1000, sf::Vector2f(700, 120), sf::Vector2f(GUI_MARGIN, win_height - 120 - GUI_MARGIN));
	fitness_graph.color = sf::Color(96, 211, 148);

	sf::Font font;
	font.loadFromFile("../font.ttf");
	sf::Text generation_text;
	sf::Text best_score_text;
	generation_text.setFont(font);
	generation_text.setCharacterSize(42);
	generation_text.setFillColor(sf::Color::White);
	generation_text.setPosition(GUI_MARGIN * 2.0f, GUI_MARGIN);

	best_score_text = generation_text;
	best_score_text.setCharacterSize(32);
	best_score_text.setPosition(4.0f * GUI_MARGIN, 64);
	
	const uint32_t pop_size = 1600;
	Stadium stadium(pop_size, 2.0f * sf::Vector2f(win_width, win_height));
	//stadium.loadDnaFromFile("../selector_output_8.bin");

	DroneRenderer drone_renderer;
	sf::RenderStates state;
	state.transform.scale(0.5f, 0.5f);

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

			stadium.update(dt, !full_speed);

			fitness_graph.setLastValue(stadium.current_iteration.best_fitness);
			generation_text.setString("Generation " + toString(stadium.selector.current_iteration));
			best_score_text.setString("Score " + toString(stadium.current_iteration.best_fitness));

			// Render
			window.clear();
			window.draw(generation_text);
			window.draw(best_score_text);

			uint32_t current_drone_i = 0;
			if (draw_drones) {
				if (show_just_one) {
					const Drone& d = stadium.selector.getCurrentPopulation()[stadium.current_iteration.best_unit];
					drone_renderer.draw(d, window, state, colors[d.index%colors.size()], !full_speed);
				}
				else {
					for (Drone& d : population) {
						if (d.alive) {
							drone_renderer.draw(d, window, state, colors[d.index%colors.size()], false);
						}
					}
				}
			}
			
			if (show_just_one) {
				sf::CircleShape target_c(target_radius);
				target_c.setFillColor(sf::Color(255, 128, 0));
				target_c.setOrigin(target_radius, target_radius);
				target_c.setPosition(stadium.targets[stadium.objectives[stadium.current_iteration.best_unit].target_id]);
				window.draw(target_c, state);
			}

			if (draw_fitness) {
				fitness_graph.render(window);
			}

			window.display();
		}

		//stadium.finalizeFitness();
		
		fitness_graph.next();
		stadium.nextIteration();
	}

	return 0;
}