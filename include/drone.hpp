#pragma once

#include "ai_unit.hpp"
#include "utils.hpp"
#include <SFML/Graphics.hpp>


const std::vector<uint64_t> architecture = { 6, 12, 8, 4 };


struct Drone : public AiUnit
{
	struct Thruster
	{
		float angle;
		float power;

		Thruster()
			: angle(0.0f)
			, power(0.0f)
		{}
	};

	Thruster left, right;
	float thruster_offset = 10.0f;
	float radius;
	sf::Vector2f position;
	sf::Vector2f velocity;
	float angle;
	float angular_velocity;

	Drone()
		: AiUnit(architecture)
		, radius(20.0f)
		, position(0.0f, 0.0f)
	{

	}

	Drone(const sf::Vector2f& pos)
		: AiUnit(architecture)
		, radius(20.0f)
		, position(pos)
	{
	}

	void reset()
	{
		velocity = sf::Vector2f(0.0f, 0.0f);
		angle = 0.0f;
		angular_velocity = 0.0f;
		left.power = 0.0f;
		left.angle = 0.0f;
		right.power = 0.0f;
		right.angle = 0.0f;

		fitness = 0.0f;
		alive = true;
	}

	sf::Vector2f getThrust() const
	{
		const float angle_left = angle + left.angle - HalfPI;
		sf::Vector2f thrust_left = left.power * sf::Vector2f(cos(angle_left), sin(angle_left));
		const float angle_right = angle + right.angle - HalfPI;
		sf::Vector2f thrust_right = right.power * sf::Vector2f(cos(angle_right), sin(angle_right));

		return thrust_left + thrust_right;
	}

	static float cross(sf::Vector2f v1, sf::Vector2f v2)
	{
		return v1.x * v2.y - v1.y * v2.x;
	}

	static float dot(sf::Vector2f v1, sf::Vector2f v2)
	{
		return v1.x * v2.x + v1.y * v2.y;
	}

	float getTorque() const
	{
		const float left_torque = left.power / thruster_offset * cos(left.angle);
		const float right_torque = -right.power / thruster_offset * cos(right.angle);
		return left_torque + right_torque;
	}

	void update(float dt)
	{
		const sf::Vector2f gravity(0.0f, 100.0f);
		// Integration
		velocity += (gravity + getThrust()) * dt;
		position += velocity * dt;
		angular_velocity += getTorque() * dt;
		angle += angular_velocity * dt;
	}

	void draw(sf::RenderTarget& target)
	{
		constexpr float RAD_TO_DEG = 57.2958f;

		// Draw body
		sf::CircleShape c(radius);
		c.setOrigin(radius, radius);
		c.setPosition(position);
		c.setFillColor(sf::Color::Green);
		target.draw(c);

		// Draw thrusters
		const float thruster_width = 10.0f;
		const float thruster_height = 30.0f;
		sf::RectangleShape thruster(sf::Vector2f(thruster_width, thruster_height));
		thruster.setOrigin(thruster_width * 0.5f, thruster_width * 0.5f);
		thruster.setFillColor(sf::Color::Green);
		const float ca = cos(angle);
		const float sa = sin(angle);
		// Left
		sf::Vector2f left_position = position - (radius + thruster_offset) * sf::Vector2f(ca, sa);
		thruster.setPosition(left_position);
		thruster.setRotation(RAD_TO_DEG * (left.angle + angle));
		target.draw(thruster);
		// Right
		sf::Vector2f right_position = position + (radius + thruster_offset) * sf::Vector2f(ca, sa);
		thruster.setPosition(right_position);
		thruster.setRotation(RAD_TO_DEG * (right.angle + angle));
		target.draw(thruster);
		// Draw thrusters power
		const float power_width = 0.5f * thruster_width;
		sf::RectangleShape power(sf::Vector2f(power_width, 1.0f));
		power.setOrigin(power_width * 0.5f, 0.0f);
		power.setFillColor(sf::Color::Red);
		// Left
		power.setScale(1.0f, left.power);
		power.setPosition(left_position);
		power.setRotation(RAD_TO_DEG * (left.angle + angle));
		target.draw(power);
		// Right
		power.setScale(1.0f, right.power);
		power.setPosition(right_position);
		power.setRotation(RAD_TO_DEG * (right.angle + angle));
		target.draw(power);
	}

	void process(const std::vector<float>& outputs) override
	{
		const float max_power = 100.0f;
		const float max_angle = 2.0f * PI;

		left.power  = max_power * outputs[0];
		left.angle  = max_angle * (outputs[1] - 0.5f);
		right.power = max_power * outputs[2];
		right.angle = max_angle * (outputs[3] - 0.5f);
	}
};
