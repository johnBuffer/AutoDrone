#pragma once

#include "selector.hpp"


struct Athlete
{
	
};


struct Challenge
{

};


template <typename T>
struct Stadium
{
	Stadium(const uint32_t athletes_count)
		: selector(athletes_count)
	{

	}

	Challenge challenge;
	Selector<T> selector;
};
