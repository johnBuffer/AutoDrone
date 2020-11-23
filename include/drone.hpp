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
		const float inertia_coef = 0.25;
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

	void draw(sf::RenderTarget& target, sf::Color color = sf::Color::White)
	{
		constexpr float RAD_TO_DEG = 57.2958f;

		// Draw body
		sf::CircleShape c(radius);
		c.setOrigin(radius, radius);
		c.setPosition(position);
		c.setFillColor(color);
		target.draw(c);

		// Draw thrusters
		const float thruster_width = 10.0f;
		const float thruster_height = 30.0f;
		sf::RectangleShape thruster(sf::Vector2f(thruster_width, thruster_height));
		thruster.setOrigin(thruster_width * 0.5f, thruster_width * 0.5f);
		thruster.setFillColor(color);
		const float ca = cos(angle);
		const float sa = sin(angle);
		// Left
		sf::Vector2f left_position = position - (radius + thruster_offset) * sf::Vector2f(ca, sa);
		thruster.setPosition(left_position);
		thruster.setRotation(RAD_TO_DEG * (angle + left.angle));
		target.draw(thruster);
		// Right
		sf::Vector2f right_position = position + (radius + thruster_offset) * sf::Vector2f(ca, sa);
		thruster.setPosition(right_position);
		thruster.setRotation(RAD_TO_DEG * (angle - right.angle));
		target.draw(thruster);
		// Draw thrusters power
		const float power_width = 0.5f * thruster_width;
		sf::RectangleShape power(sf::Vector2f(power_width, 1.0f));
		power.setOrigin(power_width * 0.5f, 0.0f);
		power.setFillColor(sf::Color::Red);
		// Left
		const float max_power_length = 100.0f;
		power.setScale(1.0f, left.power / max_power * max_power_length);
		power.setPosition(left_position);
		power.setRotation(RAD_TO_DEG * (angle + left.angle));
		target.draw(power);
		// Right
		power.setScale(1.0f, right.power / max_power * max_power_length);
		power.setPosition(right_position);
		power.setRotation(RAD_TO_DEG * (angle - right.angle));
		target.draw(power);
	}

	void process(const std::vector<float>& outputs) override
	{
		const float max_angle = PI;

		left.power  = max_power * outputs[0];
		left.angle  = max_angle * normalize(outputs[1], 1.0f);
		right.power = max_power * outputs[2];
		right.angle = max_angle * normalize(outputs[3], 1.0f);
	}
};
