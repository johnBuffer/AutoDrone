#include <SFML/Graphics.hpp>
#include <vector>
#include <list>
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
#include "dna_loader.hpp"
#include "dynamic_blur.hpp"


struct Conf
{
	float gravity_x, gravity_y;
	float max_power;
};

Conf loadConf()
{
	Conf conf;
	conf.gravity_x = 0.0f;
	conf.gravity_y = 1000.0f;
	conf.max_power = 1500.0f;

	std::ifstream conf_file("conf.txt");
	if (conf_file) {
		conf_file >> conf.gravity_x;
		conf_file >> conf.gravity_y;
		conf_file >> conf.max_power;
	}
	else {
		std::cout << "Couldn't find 'conf.txt', loading default" << std::endl;
	}

	return conf;
}


int main()
{
	NumberGenerator<>::initialize();

	const uint32_t win_width = 1920;
	const uint32_t win_height = 1080;
	sf::ContextSettings settings;
	settings.antialiasingLevel = 4;
	sf::RenderWindow window(sf::VideoMode(win_width, win_height), "AutoDrone", sf::Style::Default, settings);
	window.setFramerateLimit(144);
	window.setMouseCursorVisible(false);

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
	
	DroneRenderer drone_renderer;
	sf::RenderStates state;

	sf::RenderTexture blur_target;
	blur_target.create(win_width, win_height);

	const uint64_t dna_bytes_count = Network::getParametersCount(architecture) * 4;

	uint32_t pop_size = 1;
	Stadium stadium(pop_size, sf::Vector2f(win_width, win_height));
	event_manager.addKeyPressedCallback(sf::Keyboard::M, [&](sfev::CstEv) { stadium.use_manual_target = !stadium.use_manual_target; });

	uint32_t current_drone = 0;

	// Initialize drones
	std::vector<Drone>& population = stadium.selector.getCurrentPopulation();
	stadium.initializeIteration();

	Conf conf = loadConf();

	uint32_t i(0);
	/*DNA dna(dna_bytes_count * 8);
	dna.code = { 156, 152, 54, 192, 55, 48, 42, 193, 2, 119, 14, 192, 140, 66, 57, 63, 85, 239, 188, 192, 183, 155, 161, 
		         65, 15, 97, 8, 193, 190, 33, 13, 193, 155, 109, 45, 64, 30, 110, 137, 64, 131, 19, 73, 64, 218, 77, 226, 
		         192, 213, 38, 197, 64, 138, 36, 189, 65, 251, 93, 100, 64, 120, 21, 25, 63, 96, 177, 18, 65, 104, 66, 39, 
		         64, 245, 210, 30, 192, 53, 175, 45, 64, 124, 58, 177, 192, 54, 21, 132, 192, 177, 177, 247, 64, 32, 92, 2, 
		         64, 197, 12, 241, 64, 135, 202, 210, 188, 5, 205, 225, 190, 86, 196, 19, 64, 76, 39, 195, 63, 90, 174, 54, 
		         66, 110, 192, 246, 60, 121, 197, 13, 66, 169, 121, 33, 190, 22, 251, 21, 192, 136, 110, 68, 191, 43, 20, 14, 
		         192, 54, 137, 212, 193, 134, 44, 173, 63, 200, 129, 238, 64, 52, 155, 145, 63, 72, 94, 246, 190, 115, 43, 56, 
		         193, 139, 84, 13, 193, 42, 174, 161, 64, 93, 153, 138, 192, 232, 60, 104, 64, 185, 130, 94, 63, 182, 197, 112,
		         191, 31, 208, 35, 64, 28, 123, 232, 191, 88, 160, 204, 64, 209, 100, 94, 192, 255, 113, 14, 63, 155, 92, 213,
		         63, 219, 90, 103, 192, 38, 67, 63, 193, 62, 42, 50, 192, 116, 210, 215, 191, 91, 169, 7, 193, 29, 112, 171, 64,
		         251, 148, 197, 64, 67, 21, 29, 193, 50, 184, 72, 193, 127, 178, 26, 193, 228, 120, 230, 192, 86, 53, 56, 193,
		         154, 100, 147, 193, 250, 61, 54, 63, 208, 162, 187, 63, 213, 123, 41, 192, 63, 192, 100, 64, 171, 234, 29, 65,
		         213, 213, 230, 64, 57, 52, 101, 65, 198, 180, 22, 65, 131, 73, 215, 64, 50, 173, 179, 191, 95, 16, 20, 193, 55,
		         165, 27, 193, 180, 249, 91, 193, 107, 122, 101, 192, 50, 61, 6, 193, 150, 118, 58, 191, 252, 155, 23, 65, 27,
		         248, 178, 64, 160, 93, 237, 192, 142, 84, 158, 192, 236, 251, 245, 192, 99, 148, 193, 64, 50, 126, 248, 191, 87,
		         183, 204, 64, 68, 100, 3, 65, 150, 78, 246, 192, 180, 199, 32, 65, 128, 192, 185, 64, 94, 101, 49, 62, 15, 127,
		         136, 63, 234, 30, 144, 64, 63, 241, 131, 193, 172, 210, 84, 191, 57, 114, 134, 192, 121, 0, 199, 191, 209, 157,
		         23, 65, 122, 81, 41, 192, 145, 216, 96, 64, 157, 25, 7, 193, 7, 202, 80, 190, 168, 12, 184, 64, 234, 102, 240,
		         192, 158, 152, 1, 64, 229, 49, 32, 193, 120, 21, 36, 64, 188, 79, 157, 192, 36, 40, 170, 64, 181, 117, 12, 66,
		         183, 169, 57, 192, 212, 92, 93, 192, 183, 214, 137, 64, 121, 26, 80, 192, 140, 58, 180, 65, 216, 140, 252, 64,
		         83, 226, 3, 192, 188, 96, 149, 64, 11, 99, 235, 64, 207, 115, 204, 64, 122, 217, 34, 65, 24, 164, 42, 192, 201,
		         128, 56, 64, 17, 51, 11, 192, 20, 240, 249, 192, 106, 108, 248, 190, 63, 172, 130, 64, 255, 199, 89, 192, 229,
		         205, 143, 64, 151, 125, 176, 63, 60, 9, 22, 65, 74, 228, 129, 64, 110, 53, 76, 64, 60, 81, 141, 192, 33, 172,
		         177, 64, 209, 80, 160, 192, 16, 252, 21, 65, 181, 63, 229, 191, 73, 110, 17, 65, 102, 145, 188, 192, 5, 243,
		         58, 192, 245, 86, 202, 191, 91, 133, 14, 65, 110, 45, 148, 64, 95, 63, 35, 65, 50, 103, 131, 192, 237, 211,
		         223, 192, 158, 27, 31, 65, 102, 119, 233, 192, 245, 115, 144, 64, 112, 230, 77, 64, 197, 194, 206, 192, 142,
		         234, 34, 62, 205, 111, 208, 63, 118, 111, 118, 64, 206, 184, 169, 64, 116, 69, 140, 64, 27, 161, 88, 192,
		         181, 231, 119, 190, 203, 155, 46, 191, 63, 101, 128, 191, 156, 182, 132, 62, 48, 240, 185, 64, 216, 49, 211,
		         191, 111, 84, 32, 191, 194, 106, 54, 195, 20, 215, 197, 192, 164, 12, 20, 192, 149, 212, 112, 192, 173, 238,
		         202, 62, 24, 60, 243, 63, 232, 196, 144, 64, 118, 18, 71, 192, 215, 132, 223, 64, 204, 116, 223, 192, 49, 127,
		         38, 192, 230, 242, 18, 193, 63, 121, 131, 65, 9, 43, 224, 64, 4, 17, 157, 190, 51, 60, 222, 193, 21, 126, 36, 64,
		         203, 17, 197, 191, 111, 127, 38, 64, 173, 254, 222, 63, 207, 47, 51, 192, 214, 153, 120, 64, 40, 73, 124, 191,
		         65, 85, 130, 191, 246, 166, 193, 192, 68, 49, 210, 192, 204, 236, 85, 191, 65, 208, 46, 63, 21, 119, 52, 192,
		         6, 101, 180, 191, 57, 77, 26, 193, 32, 152, 17, 192
	};*/

	DNA dna = DnaLoader::loadDnaFrom("../selector_output_7.bin", dna_bytes_count, 440);
	std::cout << DnaLoader::getDnaCount("../selector_output_7.bin", dna_bytes_count) << std::endl;

	for (Drone& d : stadium.selector.getCurrentPopulation()) {
		d.loadDNA(dna);
		d.generation = 5500;
		d.index = i++;
		d.gravity.x = conf.gravity_x;
		d.gravity.y = conf.gravity_y;
		d.left.setMaxPower(conf.max_power);
		d.right.setMaxPower(conf.max_power);
	}

	stadium.use_manual_target = false;

	sf::VertexArray va_target(sf::LineStrip, 0);
	sf::VertexArray va_drone(sf::LineStrip, 0);
	const float new_point_distance = 8.0f;
	sf::Vector2f last_target(0.0f, 0.0f);
	sf::Vector2f last_drone;
	bool draw_histo = false;

	bool full_speed = false;
	event_manager.addKeyPressedCallback(sf::Keyboard::C, [&](sfev::CstEv ev) { va_target.clear(); va_drone.clear(); });
	event_manager.addKeyPressedCallback(sf::Keyboard::H, [&](sfev::CstEv ev) { draw_histo = !draw_histo; });
	event_manager.addKeyPressedCallback(sf::Keyboard::E, [&](sfev::CstEv ev) { full_speed = !full_speed; window.setFramerateLimit(!full_speed * 144); });

	sf::Clock clock;
	while (window.isOpen()) {
		event_manager.processEvents();

		stadium.current_iteration.time = 0.0f;

		stadium.initializeDrones();

		va_target.clear();
		va_drone.clear();

		Drone& drone = stadium.selector.getCurrentPopulation()[0];

		while (stadium.getAliveCount() && window.isOpen())
		{
			event_manager.processEvents();
			const sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);
			
			stadium.current_iteration.time += dt;

			if (stadium.use_manual_target) {
				stadium.manual_target.x = mouse_pos.x;
				stadium.manual_target.y = mouse_pos.y;

				if (getLength(last_target - stadium.manual_target) > new_point_distance) {
					last_target = stadium.manual_target;
					va_target.append(sf::Vertex(last_target, sf::Color(255, 128, 0)));
				}
			}

			stadium.update(dt, true);

			if (draw_histo) {
				if (getLength(last_drone - drone.position) > new_point_distance) {
					last_drone = drone.position;
					va_drone.append(sf::Vertex(last_drone, colors[drone.index]));
				}
			}

			// Render
			window.clear();
			blur_target.clear();

			if (draw_histo) {
				window.draw(va_target, state);
				window.draw(va_drone, state);
			}

			for (Drone& d : stadium.selector.getCurrentPopulation()) {
				if (d.alive) {
					drone_renderer.draw(d, window, blur_target, state, colors[d.index%colors.size()], true);
				}
			}

			if (stadium.use_manual_target) {
				const float target_radius = 8.0f;
				sf::CircleShape target_c(target_radius);
				target_c.setFillColor(sf::Color(255, 128, 0));
				target_c.setOrigin(target_radius, target_radius);
				target_c.setPosition(stadium.manual_target);
				window.draw(target_c);
				target_c.setFillColor(sf::Color::Black);
				blur_target.draw(target_c);
			}

			/*blur_target.display();
			sf::Sprite bloom_sprite = blur.apply(blur_target.getTexture(), 2);
			window.draw(bloom_sprite, sf::BlendAdd);*/

			window.display();
		}
	}

	return 0;
}