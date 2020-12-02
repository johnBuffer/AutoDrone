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

	sf::RenderTexture blur_target;
	blur_target.create(win_width, win_height);

	bool show_just_one = false;
	bool full_speed = false;
	bool draw_neural = false;
	bool draw_drones = true;
	bool draw_fitness = true;

	bool boost_left = false;
	bool boost_right = false;
	float left_angle = 0.0f;
	float right_angle = 0.0f;
	float booster_power_left = 0.1f;
	float booster_power_right = 0.1f;

	Drone drone;
	drone.reset();

	uint32_t current_color = 0;

	event_manager.addKeyReleasedCallback(sf::Keyboard::LShift, [&](sfev::CstEv ev) { ++current_color; });

	event_manager.addKeyPressedCallback(sf::Keyboard::Space, [&](sfev::CstEv ev) { boost_left = true; });
	event_manager.addKeyReleasedCallback(sf::Keyboard::Space, [&](sfev::CstEv ev) { boost_left = false; });

	event_manager.addKeyPressedCallback(sf::Keyboard::Numpad0, [&](sfev::CstEv ev) { boost_right = true; });
	event_manager.addKeyReleasedCallback(sf::Keyboard::Numpad0, [&](sfev::CstEv ev) { boost_right = false; });

	event_manager.addKeyPressedCallback(sf::Keyboard::D, [&](sfev::CstEv ev) { left_angle += 0.05f;  drone.left.setAngle(left_angle); });
	event_manager.addKeyPressedCallback(sf::Keyboard::Q, [&](sfev::CstEv ev) { left_angle -= 0.05f;  drone.left.setAngle(left_angle); });

	event_manager.addKeyPressedCallback(sf::Keyboard::Up, [&](sfev::CstEv ev) { booster_power_right += 0.1f; });
	event_manager.addKeyPressedCallback(sf::Keyboard::Down, [&](sfev::CstEv ev) { booster_power_right -= 0.1f; });

	event_manager.addKeyPressedCallback(sf::Keyboard::Z, [&](sfev::CstEv ev) { booster_power_left += 0.1f; });
	event_manager.addKeyPressedCallback(sf::Keyboard::S, [&](sfev::CstEv ev) { booster_power_left -= 0.1f; });

	event_manager.addKeyPressedCallback(sf::Keyboard::Left, [&](sfev::CstEv ev) { right_angle += 0.05f;  drone.right.setAngle(right_angle); });
	event_manager.addKeyPressedCallback(sf::Keyboard::Right, [&](sfev::CstEv ev) { right_angle -= 0.05f;  drone.right.setAngle(right_angle); });

	DroneRenderer drone_renderer;

	sf::Clock clock;
	while (window.isOpen()) {
		event_manager.processEvents();

		drone.left.setPower(booster_power_left * boost_left);
		drone.right.setPower(booster_power_right * boost_right);

		drone.update(dt, true);
		// Render
		window.clear();
		blur_target.clear();

		const float scale = 3.0f;
		sf::RenderStates state;
		state.transform.scale(scale, scale);
		drone_renderer.draw(drone, window, blur_target, state, colors[3], !full_speed);

		blur_target.display();
		sf::Sprite bloom_sprite = blur.apply(blur_target.getTexture(), 2);

		window.draw(bloom_sprite, sf::BlendAdd);

		window.display();
	}

	return 0;
}