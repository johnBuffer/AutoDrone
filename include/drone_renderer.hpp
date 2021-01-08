#pragma once
#include <SFML/Graphics.hpp>
#include "drone.hpp"
#include "resource_manager.hpp"


struct DroneRenderer
{
	sf::Texture flame;
	sf::Sprite flame_sprite;
	sf::Texture smoke;
	sf::Sprite smoke_sprite;
	
	DroneRenderer()
	{
		BaseManager::RegisterTexture("flame.png", "flame");
		BaseManager::RegisterTexture("smoke.png", "smoke");
		flame_sprite = BaseManager::CreateSprite("flame", sf::Vector2f(0.15f, 0.15f), sf::Vector2f(118.0f, 67.0f));
		smoke_sprite = BaseManager::CreateSprite("smoke", sf::Vector2f(1.0f, 1.0f), sf::Vector2f(126.0f, 134.0f));
	}

	void draw(const Drone::Thruster& thruster, const Drone& drone, sf::RenderTarget& target, sf::Color color, bool right, const sf::RenderStates& state)
	{
		const float offset_dir = (right ? 1.0f : -1.0f);

		const float thruster_width = 14.0f;
		const float thruster_height = 32.0f;
		const float ca = cos(drone.angle);
		const float sa = sin(drone.angle);
		const float angle = drone.angle + offset_dir * thruster.angle;
		const float ca_left = cos(angle + HalfPI);
		const float sa_left = sin(angle + HalfPI);
		const sf::Vector2f drone_dir = sf::Vector2f(ca, sa);
		const sf::Vector2f drone_normal = sf::Vector2f(-sa, ca);
		const sf::Vector2f thruster_dir = sf::Vector2f(ca_left, sa_left);
		const sf::Vector2f thruster_normal = sf::Vector2f(-sa_left, ca_left);
		sf::Vector2f position = drone.position + offset_dir * (drone.radius + drone.thruster_offset) * drone_dir;

		// Draw piston
		const sf::Vector2f on_thruster_position = position + (offset_dir * thruster_width * 0.4f) * thruster_normal - 7.0f * thruster_dir;
		const sf::Vector2f on_body_position = drone.position + 20.0f * offset_dir * drone_dir - 12.0f * drone_normal;
		sf::Color push_color(100, 100, 100);
		const sf::Vector2f body_to_thruster = on_thruster_position - on_body_position;
		const float push_length = getLength(body_to_thruster);
		const float push_angle = getAngle(body_to_thruster) + PI;
		const float push_width = 22.0f;
		const float push_height = 4.0f;
		sf::RectangleShape push(sf::Vector2f(1.0f, push_height));
		push.setOrigin(1.0f, push_height * 0.5f);
		push.setScale(push_width, 1.0f);
		push.setPosition(on_body_position);
		push.setRotation(push_angle * RAD_TO_DEG);
		push.setFillColor(push_color);
		target.draw(push, state);
		push.setScale(push_length, 0.5f);
		target.draw(push, state);

		const sf::Vector2f on_thruster_position_2 = position + (offset_dir * thruster_width * 0.4f) * thruster_normal + 8.0f * thruster_dir;
		const sf::Vector2f on_body_position_2 = drone.position + 10.0f * offset_dir * drone_dir + 10.0f * drone_normal;
		const sf::Vector2f body_to_thruster_2 = on_thruster_position_2 - on_body_position_2;
		const float push_length_2 = getLength(body_to_thruster_2);
		const float push_angle_2 = getAngle(body_to_thruster_2) + PI;

		push.setScale(push_width * 1.25f, 1.0f);
		push.setPosition(on_body_position_2);
		push.setRotation(push_angle_2 * RAD_TO_DEG);
		target.draw(push, state);
		push.setScale(push_length_2, 0.5f);
		target.draw(push, state);

		// Draw flame
		const float rand_pulse_left = (1.0f + rand() % 10 * 0.05f);
		const float v_scale_left = thruster.power_ratio * rand_pulse_left;
		flame_sprite.setPosition(position + 0.5f * thruster_height * thruster_dir);
		flame_sprite.setScale(0.15f * thruster.power_ratio * rand_pulse_left, 0.15f * v_scale_left);
		flame_sprite.setRotation(RAD_TO_DEG * angle);
		target.draw(flame_sprite, state);

		sf::RectangleShape thruster_body(sf::Vector2f(thruster_width, thruster_height));
		thruster_body.setOrigin(thruster_width * 0.5f, thruster_height * 0.5f);
		thruster_body.setFillColor(color);
		thruster_body.setPosition(position);
		thruster_body.setRotation(RAD_TO_DEG * angle);
		target.draw(thruster_body, state);

		const float margin = 2.0f;
		const float width = thruster_width * 0.5f;
		const float height = (thruster_height - 11.0f * margin) * 0.1f;
		sf::RectangleShape power_indicator(sf::Vector2f(width, height));
		power_indicator.setFillColor(sf::Color::Green);
		power_indicator.setOrigin(width * 0.5f, height);
		power_indicator.setRotation(angle * RAD_TO_DEG);
		const uint8_t power_percent = static_cast<uint8_t>(thruster.power_ratio * 10);
		sf::Vector2f power_start(position + (0.5f * thruster_height - margin) * thruster_dir);
		for (uint8_t i(0); i < power_percent; ++i) {
			power_indicator.setPosition(power_start - float(i) * (height + margin) * thruster_dir);
			target.draw(power_indicator, state);
		}
	}

	void draw(const Drone& drone, sf::RenderTarget& target, const sf::RenderStates& state, sf::Color color = sf::Color::White, bool draw_smoke = true)
	{
		// Draw body
		const float drone_width = drone.radius + drone.thruster_offset;
		sf::RectangleShape lat(sf::Vector2f(2.0f * drone_width, 6.0f));
		lat.setOrigin(drone_width, 2.0f);
		lat.setPosition(drone.position);
		lat.setRotation(RAD_TO_DEG * drone.angle);
		lat.setFillColor(sf::Color(70, 70, 70));
		target.draw(lat, state);

		draw(drone.left, drone, target, color, false, state);
		draw(drone.right, drone, target, color, true, state);
		drawBody(drone, color, target, state);
	}

	sf::Color getRedGreenRatio(float ratio) const
	{
		const float r = std::min(1.0f, std::abs(ratio));
		return sf::Color(static_cast<uint8_t>(255 * r), static_cast<uint8_t>(255 * (1.0f - r)), 0);
	}

	void drawBody(const Drone& drone, const sf::Color& color, sf::RenderTarget& target, const sf::RenderStates& state)
	{
		const float angle_ratio = std::min(1.0f, std::abs(float(sin(drone.angle))));
		const sf::Color eye_color = getRedGreenRatio(angle_ratio);

		const float r = drone.radius * 1.25f;
		const uint32_t quality = 24;
		sf::VertexArray va(sf::TriangleFan, quality + 3);
		const float da = 2.0f * PI / float(quality);
		va[0].position = drone.position;
		va[0].color = eye_color;
		for (uint32_t i(1); i < quality / 2 + 2; ++i) {
			const float angle = drone.angle - (i - 1) * da;
			va[i].position = drone.position + r * sf::Vector2f(cos(angle), sin(angle));
			va[i].color = color;
		}
		for (uint32_t i(quality / 2 + 1); i < quality + 2; ++i) {
			const float angle = drone.angle - (i - 1) * da;
			va[i + 1].position = drone.position + 0.65f * r * sf::Vector2f(cos(angle), sin(angle));
			va[i + 1].color = sf::Color(120, 120, 120);
		}

		target.draw(va, state);

		const float inner_r = drone.radius * 0.35f;
		sf::CircleShape c(inner_r);
		c.setOrigin(inner_r, inner_r);
		c.setPosition(drone.position);
		c.setFillColor(eye_color);
		target.draw(c, state);

		const float led_size = 2.0f;
		const float led_offset = 12.0f;
		sf::CircleShape c_led(led_size);
		c_led.setOrigin(led_size + led_offset, led_size);
		c_led.setRotation(drone.angle * RAD_TO_DEG);
		c_led.setPosition(drone.position);
		c_led.setFillColor(getRedGreenRatio(drone.left.angle_ratio));
		target.draw(c_led, state);

		c_led.setOrigin(led_size - led_offset, led_size);
		c_led.setFillColor(getRedGreenRatio(drone.right.angle_ratio));
		target.draw(c_led, state);

		c_led.setOrigin(led_size, led_size - 0.45f * r);
		c_led.setFillColor(getRedGreenRatio(1.0f - drone.network.last_input[0]));
		target.draw(c_led, state);

		c_led.setOrigin(led_size, led_size + 0.5f * r);
		c_led.setFillColor(getRedGreenRatio(1.0f - drone.network.last_input[1]));
		target.draw(c_led, state);
	}
};
