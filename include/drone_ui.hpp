#pragma once
#include <SFML/Graphics.hpp>
#include "drone.hpp"


struct ThrusterUI : sf::Drawable
{
	// Link to the underlying objects
	const Drone::Thruster& thruster;
	// The sprite provided by the renderer
	sf::Texture& flame;
	// Attributes
	float width  = 14.0f;
	float height = 32.0f;

	sf::VertexArray va;

	ThrusterUI(const Drone::Thruster& thruster_, sf::Texture& flame_)
		: thruster(thruster_)
		, flame(flame_)
		, va(sf::Quads, 0)
	{}

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override
	{
		// Draw flame
		//const float rand_pulse_left = getFastRandUnder(0.5f); //(1.0f + rand() % 10 * 0.05f);
		//const float v_scale_left = thruster.power_ratio * rand_pulse_left;
		////flame_sprite.setScale(0.15f * thruster.power_ratio * rand_pulse_left, 0.15f * v_scale_left);
		//va.append(sf::Vertex(sf::Vector2f(-width * 0.5f, height * 0.5f), ))

		//sf::RectangleShape thruster_body(sf::Vector2f(thruster_width, thruster_height));
		//thruster_body.setOrigin(thruster_width * 0.5f, thruster_height * 0.5f);
		//thruster_body.setFillColor(color);
		//thruster_body.setPosition(position);
		//thruster_body.setRotation(RAD_TO_DEG * angle);
		//target.draw(thruster_body, state);

		//const float margin = 2.0f;
		//const float width = thruster_width * 0.5f;
		//const float height = (thruster_height - 11.0f * margin) * 0.1f;
		//sf::RectangleShape power_indicator(sf::Vector2f(width, height));
		//power_indicator.setFillColor(sf::Color::Green);
		//power_indicator.setOrigin(width * 0.5f, height);
		//power_indicator.setRotation(angle * RAD_TO_DEG);
		//const uint8_t power_percent = thruster.power_ratio * 10;
		//sf::Vector2f power_start(position + (0.5f * thruster_height - margin) * thruster_dir);
		//for (uint8_t i(0); i < power_percent; ++i) {
		//	power_indicator.setPosition(power_start - float(i) * (height + margin) * thruster_dir);
		//	target.draw(power_indicator, state);
		//}
	}
};

struct DroneUI
{

};
