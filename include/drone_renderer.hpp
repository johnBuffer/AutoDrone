#pragma once
#include <SFML/Graphics.hpp>
#include "drone.hpp"

constexpr float RAD_TO_DEG = 57.2958f;


struct DroneRenderer
{
	sf::Texture flame;
	sf::Sprite flame_sprite;
	DroneRenderer()
	{
		flame.loadFromFile("../flame.png");
		flame_sprite.setTexture(flame);
		flame_sprite.setOrigin(118.0f, 67.0f);
		flame_sprite.setScale(0.15f, 0.15f);
	}

	void draw(const Drone::Thruster& thruster, const Drone& drone, sf::RenderTarget& target, sf::Color color, bool right)
	{
		const float offset_dir = (right ? 1.0f : -1.0f);
		const float power_ratio = drone.left.power / drone.max_power;

		const float thruster_width = 12.0f;
		const float thruster_height = 32.0f;
		const float ca = cos(drone.angle);
		const float sa = sin(drone.angle);
		const float angle = drone.angle + offset_dir * thruster.angle;
		const float ca_left = cos(angle + HalfPI);
		const float sa_left = sin(angle + HalfPI);
		const sf::Vector2f drone_dir = sf::Vector2f(ca, sa);
		const sf::Vector2f drone_normal = sf::Vector2f(-sa, ca);
		const sf::Vector2f thruster_dir = sf::Vector2f(ca_left, sa_left);
		sf::Vector2f position = drone.position  + offset_dir * (drone.radius + drone.thruster_offset) * drone_dir;

		const float rand_pulse_left = (1.0f + rand() % 10 * 0.05f);
		const float v_scale_left = power_ratio * rand_pulse_left;
		flame_sprite.setPosition(position + 0.5f * thruster_height * sf::Vector2f(ca_left, sa_left));
		flame_sprite.setScale(0.15f * power_ratio * rand_pulse_left, 0.15f * v_scale_left);
		flame_sprite.setRotation(RAD_TO_DEG * angle);
		target.draw(flame_sprite);

		sf::VertexArray va(sf::Lines, 2);
		va[0].position = drone.position - 10.0f * drone_normal;
		va[1].position = position - 5.0f * thruster_dir;
		va[0].color = color;
		va[1].color = color;
		target.draw(va);

		sf::RectangleShape thruster_body(sf::Vector2f(thruster_width, thruster_height));
		thruster_body.setOrigin(thruster_width * 0.5f, thruster_height * 0.5f);
		thruster_body.setFillColor(color);
		thruster_body.setPosition(position);
		thruster_body.setRotation(RAD_TO_DEG * angle);
		target.draw(thruster_body);

		const float margin = 2.0f;
		const float width = thruster_width * 0.5f;
		const float height = (thruster_height - 12.0f * margin) * 0.1f;
		sf::RectangleShape power_indicator(sf::Vector2f(width, height));
		power_indicator.setFillColor(sf::Color::Green);
		power_indicator.setOrigin(width * (0.5f - 0.35f * offset_dir), height);
		power_indicator.setRotation(angle * RAD_TO_DEG);
		const uint8_t power_percent = power_ratio * 10;
		sf::Vector2f power_start(position + (0.5f * thruster_height - margin) * thruster_dir);
		for (uint8_t i(0); i < power_percent; ++i) {
			power_indicator.setPosition(power_start - float(i) * (height + margin) * thruster_dir);
			target.draw(power_indicator);
		}
	}

	void draw(const Drone& drone, sf::RenderTarget& target, sf::Color color = sf::Color::White)
	{
		// Draw body
		const float drone_width = drone.radius + drone.thruster_offset;
		sf::RectangleShape lat(sf::Vector2f(2.0f * drone_width, 4.0f));
		lat.setOrigin(drone_width, 2.0f);
		lat.setPosition(drone.position);
		lat.setRotation(RAD_TO_DEG * drone.angle);
		lat.setFillColor(color);
		target.draw(lat);
		sf::CircleShape c(drone.radius);
		c.setOrigin(drone.radius, drone.radius);
		c.setPosition(drone.position);
		c.setFillColor(color);
		target.draw(c);

		draw(drone.left, drone, target, color, false);
		draw(drone.right, drone, target, color, true);
	}
};
