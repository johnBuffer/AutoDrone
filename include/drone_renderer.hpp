#pragma once
#include <SFML/Graphics.hpp>
#include "drone.hpp"


struct DroneRenderer
{
	sf::Texture flame;
	sf::Sprite flame_sprite;
	DroneRenderer()
	{
		flame.loadFromFile("../flame.png");
		flame_sprite.setTexture(flame);
		flame_sprite.setOrigin(118.0f, 120.0f);
		flame_sprite.setScale(0.15f, 0.15f);
	}

	void draw(const Drone& drone, sf::RenderTarget& target, sf::Color color = sf::Color::White)
	{
		constexpr float RAD_TO_DEG = 57.2958f;

		// Draw body
		sf::CircleShape c(drone.radius);
		c.setOrigin(drone.radius, drone.radius);
		c.setPosition(drone.position);
		c.setFillColor(color);
		target.draw(c);

		// Draw thrusters
		const float thruster_width = 10.0f;
		const float thruster_height = 30.0f;
		sf::RectangleShape thruster(sf::Vector2f(thruster_width, thruster_height));
		thruster.setOrigin(thruster_width * 0.5f, thruster_width * 0.5f);
		thruster.setFillColor(color);
		const float ca = cos(drone.angle);
		const float sa = sin(drone.angle);
		// Left
		sf::Vector2f left_position = drone.position - (drone.radius + drone.thruster_offset) * sf::Vector2f(ca, sa);
		thruster.setPosition(left_position);
		const float left_angle = drone.angle + drone.left.angle;
		thruster.setRotation(RAD_TO_DEG * left_angle);
		target.draw(thruster);
		// Right
		sf::Vector2f right_position = drone.position + (drone.radius + drone.thruster_offset) * sf::Vector2f(ca, sa);
		thruster.setPosition(right_position);
		const float right_angle = drone.angle - drone.right.angle;
		thruster.setRotation(RAD_TO_DEG * right_angle);
		target.draw(thruster);
		// Draw thrusters power
		const float max_power_length = 100.0f;
		const float power_width = 0.5f * thruster_width;
		sf::RectangleShape power(sf::Vector2f(power_width, 1.0f));
		power.setOrigin(power_width * 0.5f, 0.0f);
		power.setFillColor(sf::Color::Red);
		// Left
		const float ca_left = cos(left_angle + HalfPI);
		const float sa_left = sin(left_angle + HalfPI);
		flame_sprite.setPosition(left_position + thruster_height * sf::Vector2f(ca_left, sa_left));
		const float rand_pulse_left = (1.0f + rand() % 10 * 0.05f);
		const float scale_left = drone.left.power / drone.max_power;
		const float v_scale_left = scale_left  * rand_pulse_left;
		flame_sprite.setScale(0.15f * scale_left * rand_pulse_left, 0.15f * v_scale_left);
		flame_sprite.setRotation(RAD_TO_DEG * left_angle);
		target.draw(flame_sprite);
		// Right
		const float ca_right = cos(right_angle + HalfPI);
		const float sa_right = sin(right_angle + HalfPI);
		flame_sprite.setPosition(right_position + thruster_height * sf::Vector2f(ca_right, sa_right));
		const float rand_pulse_right = (1.0f + rand() % 10 * 0.05f);
		const float scale_right = drone.right.power / drone.max_power;
		const float v_scale_right = scale_right * rand_pulse_right;
		flame_sprite.setScale(0.15f * scale_right * rand_pulse_right, 0.15f * v_scale_right);
		flame_sprite.setRotation(RAD_TO_DEG * right_angle);
		target.draw(flame_sprite);
	}
};
