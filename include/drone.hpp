#pragma once

#include "ai_unit.hpp"
#include "utils.hpp"
#include <SFML/Graphics.hpp>


const std::vector<uint64_t> architecture = { 7, 9, 7, 4 };


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
	float thruster_offset = 100.0f;
	float radius;
	sf::Vector2f position;
	sf::Vector2f velocity;
	float angle;
	float angular_velocity;
	float max_power = 1000.0f;

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

	static float cross(sf::Vector2f v1, sf::Vector2f v2)
	{
		return v1.x * v2.y - v1.y * v2.x;
	}

	static float dot(sf::Vector2f v1, sf::Vector2f v2)
	{
		return v1.x * v2.x + v1.y * v2.y;
	}

	sf::Vector2f getThrust() const
	{
		const float angle_left = angle + left.angle - HalfPI;
		sf::Vector2f thrust_left = left.power * sf::Vector2f(cos(angle_left), sin(angle_left));
		const float angle_right = angle - right.angle - HalfPI;
		sf::Vector2f thrust_right = right.power * sf::Vector2f(cos(angle_right), sin(angle_right));

		return thrust_left + thrust_right;
	}

	float getTorque() const
	{
		const float inertia_coef = 0.8f;
		const float angle_left = left.angle - HalfPI;
		const float left_torque = left.power / thruster_offset * cross(sf::Vector2f(cos(angle_left), sin(angle_left)), sf::Vector2f(1.0f, 0.0f));


		const float angle_right = -right.angle - HalfPI;
		const float right_torque = right.power / thruster_offset * cross(sf::Vector2f(cos(angle_right), sin(angle_right)), sf::Vector2f(-1.0f, 0.0f));
		return (left_torque + right_torque) * inertia_coef;
	}

	void update(float dt)
	{
		const sf::Vector2f gravity(0.0f, 150.0f);
		// Integration
		velocity += (gravity + getThrust()) * dt;
		position += velocity * dt;
		angular_velocity += getTorque() * dt;
		angle += angular_velocity * dt;
	}

	float getNormalizedAngle() const
	{
		return getAngle(sf::Vector2f(cos(angle), sin(angle))) / PI;
	}

	void process(const std::vector<float>& outputs) override
	{
		const float max_angle = 0.35f * PI;

		left.power  = max_power * outputs[0];
		left.angle  = max_angle * normalize(outputs[1]-0.5f, 1.0f);
		right.power = max_power * outputs[2];
		right.angle = max_angle * normalize(outputs[3]-0.5f, 1.0f);
	}
};
