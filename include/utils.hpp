#pragma once
#include <SFML/Graphics.hpp>
#include <sstream>
#include <random>
#include <iomanip>



constexpr float PI = 3.14159265f;


void resetRand();


float getRandRange(float width);


float getRandUnder(float width);


uint32_t getIntUnder(const uint32_t max);


float getRandRange(float width, std::mt19937& generator);


float getRandUnder(float width, std::mt19937& gen);


uint32_t getIntUnder(const uint32_t max, std::mt19937& gen);


float getRandRangeNonReset(float width);


float getRandUnderNonReset(float width);


uint32_t getIntUnderNonReset(const uint32_t max);


float normalize(float value, float range);


template<typename T>
float getLength(const sf::Vector2<T>& v)
{
	return sqrt(v.x * v.x + v.y * v.y);
}


template<typename T, typename U>
T as(const U& u)
{
	return static_cast<T>(u);
}


float getAngle(const sf::Vector2f& v);


float dot(const sf::Vector2f& v1, const sf::Vector2f& v2);


float sign(const float f);


float sigm(const float f);


template<typename T>
std::string toString(const T& v, const uint8_t decimals = 2)
{
	std::stringstream sx;
	sx << std::setprecision(decimals) << std::fixed;
	sx << v;

	return sx.str();
}

sf::RectangleShape getLine(const sf::Vector2f& point_1, const sf::Vector2f& point_2, const float width, const sf::Color& color);


template<typename T>
sf::Color toColor(const sf::Vector3<T>& v)
{
	const uint8_t r = as<uint8_t>(v.x);
	const uint8_t g = as<uint8_t>(v.y);
	const uint8_t b = as<uint8_t>(v.z);
	return sf::Color(std::min(uint8_t(255), r), std::min(uint8_t(255), g), std::min(uint8_t(255), b));
}


template<typename T>
T clamp(const T& min_val, const T& max_val, const T& value)
{
	return std::min(max_val, std::max(min_val, value));
}
