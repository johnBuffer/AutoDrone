#include "utils.hpp"

#include <limits>

std::random_device rd;
std::mt19937 gen(0);
std::mt19937 gen_no_reset(rd());


void resetRand()
{
	gen = std::mt19937(0);
}

float getRandRange(float width)
{
	std::uniform_real_distribution<float> distr(-width, width);
	return distr(gen);
}

float getRandUnder(float width)
{
	std::uniform_real_distribution<float> distr(0.0f, width);
	return distr(gen);
}

uint32_t getIntUnder(const uint32_t max)
{
	std::uniform_int_distribution<std::mt19937::result_type> distr(0, max);
	return  distr(gen);
}

float getRandRange(float width, std::mt19937& generator)
{
	std::uniform_real_distribution<float> distr(-width, width);
	return distr(generator);
}

float getRandUnder(float width, std::mt19937& generator)
{
	std::uniform_real_distribution<float> distr(0.0f, width);
	return distr(generator);
}

uint32_t getIntUnder(const uint32_t max, std::mt19937& generator)
{
	std::uniform_int_distribution<std::mt19937::result_type> distr(0, max);
	return  distr(generator);
}

float getRandRangeNonReset(float width)
{
	std::uniform_real_distribution<float> distr(-width, width);
	return distr(gen_no_reset);
}

float getRandUnderNonReset(float width)
{
	std::uniform_real_distribution<float> distr(0.0f, width);
	return distr(gen_no_reset);
}

uint32_t getIntUnderNonReset(const uint32_t max)
{
	std::uniform_int_distribution<std::mt19937::result_type> distr(0, max);
	return  distr(gen_no_reset);
}

float normalize(float value, float range)
{
	return 2.0f * (value / range - 0.5f);
}

float getAngle(const sf::Vector2f & v)
{
	const float a = acos(v.x / getLength(v));
	return v.y > 0.0f ? a : -a;
}

float dot(const sf::Vector2f & v1, const sf::Vector2f & v2)
{
	return v1.x * v2.x + v1.y * v2.y;
}

float sign(const float f)
{
	return f < 0.0f ? -1.0f : 1.0f;
}

float sigm(const float f)
{
	return f / (1 + std::abs(f));
}

sf::RectangleShape getLine(const sf::Vector2f& point_1, const sf::Vector2f& point_2, const float width, const sf::Color& color)
{
	const sf::Vector2f vec = point_2 - point_1;
	const float angle = getAngle(vec);
	const sf::Vector2f mid_point = point_1 + 0.5f * vec;
	const float dist = getLength(vec);
	const float rad_to_deg = 57.2958f;

	sf::RectangleShape line(sf::Vector2f(width, dist));
	line.setOrigin(width * 0.5f, dist * 0.5f);
	line.setRotation(angle * rad_to_deg - 90);
	line.setFillColor(color);
	line.setPosition(mid_point);

	return line;
}
