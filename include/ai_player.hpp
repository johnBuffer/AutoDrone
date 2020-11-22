#pragma once
#include "ai_unit.hpp"
#include "neural_network.hpp"
#include "game.hpp"


const std::vector<uint64_t> architecture = { 3, 6, 4, 1 };


struct Agent : public AiUnit
{
	Agent()
		: AiUnit(architecture)
	{}

	Agent(const Player &player_)
		: AiUnit(architecture)
		, player(player_)
	{
	}

	void process(const std::vector<float>& outputs) override
	{
		/*if (outputs[0] > 0.5f) {
			player.jump();
		}*/
		const float max_power = 2.0f;
		player.power = outputs[0] * max_power;
		player.fly(player.power);
	}

	Player player;
};
