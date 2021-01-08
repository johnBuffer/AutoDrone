#include <SFML/Graphics.hpp>
#include <event_manager.hpp>
#include <iostream>

#include "selector.hpp"
#include "number_generator.hpp"
#include "neural_renderer.hpp"
#include "graph.hpp"
#include "drone_renderer.hpp"
#include "stadium.hpp"
#include "resource_manager.hpp"
#include "interface_controls.hpp"


int main()
{
	NumberGenerator<>::initialize();

	const uint32_t win_width = 1920;
	const uint32_t win_height = 1080;
	sf::ContextSettings settings;
	settings.antialiasingLevel = 4;
	sf::RenderWindow window(sf::VideoMode(win_width, win_height), "AutoDrone", sf::Style::Default, settings);

	const uint64_t base_framerate(144);
	window.setFramerateLimit(base_framerate);

	sfev::EventManager event_manager(window);
	event_manager.addEventCallback(sf::Event::Closed, [&](sfev::CstEv ev) { window.close(); });
	event_manager.addKeyPressedCallback(sf::Keyboard::Escape, [&](sfev::CstEv ev) { window.close(); });
	InterfaceControls controls(event_manager, base_framerate);

	// Define constants
	const float target_radius = 8.0f;
	const float GUI_MARGIN = 10.0f;
	const float scale = 2.0f;
	const float dt = 0.008f;
	const float max_iteration_duration = 100.0f;
	const uint32_t pop_size = 800;
	const std::vector<sf::Color> colors({ sf::Color(36, 123, 160),
									sf::Color(161, 88, 86),
									sf::Color(249, 160, 97),
									sf::Color(80, 81, 79), 
		                            sf::Color(121, 85, 83),
									sf::Color(242, 95, 92),
									sf::Color(255, 224, 102),
									sf::Color(146, 174, 131),
									sf::Color(74, 158, 170),
									sf::Color(112, 193, 179) });

	
	Graphic fitness_graph(1000, sf::Vector2f(700, 120), sf::Vector2f(GUI_MARGIN, win_height - 120 - GUI_MARGIN));
	fitness_graph.color = sf::Color(96, 211, 148);

	BaseManager::Initialize("../res/");
	BaseManager::RegisterFont("font.ttf", "font");
	sf::Text generation_text = BaseManager::CreateText("font", 42);
	sf::Text best_score_text = BaseManager::CreateText("font", 32);
	generation_text.setPosition(GUI_MARGIN * 2.0f, GUI_MARGIN);
	best_score_text.setPosition(4.0f * GUI_MARGIN, 64);

	Stadium stadium(pop_size, scale * sf::Vector2f(win_width, win_height));
	//stadium.loadDnaFromFile("../selector_output_18.bin");

	sf::RenderStates state;
	DroneRenderer drone_renderer;
	state.transform.scale(1.0f / scale, 1.0f / scale);

	while (window.isOpen()) {
		event_manager.processEvents();
		
		// Check for new generation
		if (stadium.isDone()) {
			fitness_graph.next();
			stadium.newIteration();
		}
		std::vector<Drone>& population = stadium.selector.getCurrentPopulation();

		stadium.update(dt, !controls.full_speed);

		fitness_graph.setLastValue(stadium.current_iteration.best_fitness);
		generation_text.setString("Generation " + toString(stadium.selector.generation));
		best_score_text.setString("Score " + toString(stadium.current_iteration.best_fitness));

		// Render
		window.clear();
		window.draw(generation_text);
		window.draw(best_score_text);

		uint32_t current_drone_i = 0;
		if (controls.draw_drones) {
			if (controls.show_just_one) {
				const Drone& d = stadium.selector.getCurrentPopulation()[stadium.current_iteration.best_unit];
				drone_renderer.draw(d, window, state, colors[d.index%colors.size()], !controls.full_speed);
			}
			else {
				for (Drone& d : population) {
					if (d.alive) {
						drone_renderer.draw(d, window, state, colors[d.index%colors.size()], false);
					}
				}
			}
		}
			
		if (controls.show_just_one) {
			sf::CircleShape target_c(target_radius);
			target_c.setFillColor(sf::Color(255, 128, 0));
			target_c.setOrigin(target_radius, target_radius);
			target_c.setPosition(stadium.targets[stadium.objectives[stadium.current_iteration.best_unit].target_id]);
			window.draw(target_c, state);
		}

		if (controls.draw_fitness) {
			fitness_graph.render(window);
		}

		window.display();
	}

	BaseManager::Close();

	return 0;
}
