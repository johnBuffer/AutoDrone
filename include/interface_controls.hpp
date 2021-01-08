#pragma once
#include "event_manager.hpp"


struct InterfaceControls
{
	bool show_just_one = false;
	bool full_speed = false;
	bool manual_control = false;
	bool draw_neural = true;
	bool draw_drones = true;
	bool draw_fitness = true;
	uint32_t framerate;

	InterfaceControls(sfev::EventManager& manager, uint32_t base_framerate)
		: framerate(base_framerate)
	{
		registerCallbacks(manager);
	}

	void registerCallbacks(sfev::EventManager& manager)
	{
		manager.addKeyPressedCallback(sf::Keyboard::E, [&](sfev::CstEv ev) { 
			full_speed = !full_speed; 
			manager.getWindow().setFramerateLimit((!full_speed) * framerate);
		});
		manager.addKeyPressedCallback(sf::Keyboard::M, [&](sfev::CstEv ev) { manual_control = !manual_control; });
		manager.addKeyPressedCallback(sf::Keyboard::S, [&](sfev::CstEv ev) { show_just_one = !show_just_one; });
		manager.addKeyPressedCallback(sf::Keyboard::N, [&](sfev::CstEv ev) { draw_neural = !draw_neural; });
		manager.addKeyPressedCallback(sf::Keyboard::D, [&](sfev::CstEv ev) { draw_drones = !draw_drones; });
		manager.addKeyPressedCallback(sf::Keyboard::F, [&](sfev::CstEv ev) { draw_fitness = !draw_fitness; });
	}
};
