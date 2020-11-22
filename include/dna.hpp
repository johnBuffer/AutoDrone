#pragma once

#include <vector>
#include <iostream>
#include <bitset>


constexpr float MAX_RANGE = 16.0f;


struct DNA
{
	using byte = uint8_t;

	DNA(const uint64_t bits_count)
		: code(bits_count / 8u + bool(bits_count % 8 && bits_count > 8))
	{}

	template<typename T>
	void initialize(const float range)
	{
		const uint64_t element_count = getElementsCount<T>();
		for (uint64_t i(element_count - 1); i--;) {
			const T value = NumberGenerator<>::getInstance().get(range);
			set(i, value);
		}
	}

	template<typename T>
	T get(const uint64_t offset) const
	{
		T result;
		const uint64_t dna_offset = offset * sizeof(T);
		memcpy(&result, &code[dna_offset], sizeof(T));
		return result;
	}

	template<typename T>
	void set(const uint64_t offset, const T& value)
	{
		const float checked_value = clamp(-MAX_RANGE, MAX_RANGE, value);
		const uint64_t dna_offset = offset * sizeof(T);
		memcpy(&code[dna_offset], &value, sizeof(T));
	}

	uint64_t getBytesCount() const
	{
		return code.size();
	}

	template<typename T>
	uint64_t getElementsCount() const
	{
		return code.size() / sizeof(T);
	}

	void print() const
	{
		for (const byte word : code) {
			std::cout << std::bitset<8>(word) << ' ';
		}
		std::cout << std::endl;
	}

	void mutateBits(const float probability)
	{
		for (byte& b : code) {
			for (uint64_t i(0); i < 8; ++i) {
				if (NumberGenerator<>::getInstance().getUnder(1.0f) < probability) {
					const uint8_t mask = 256 >> i;
					b ^= mask;
				}
			}
		}
	}

	template<typename T>
	void mutate(const float probability)
	{
		constexpr uint32_t type_size = sizeof(T);
		const uint64_t element_count = code.size() / type_size;
		for (uint64_t i(0); i < element_count; ++i) {
			if (NumberGenerator<>::getInstance().getUnder(1.0f) < probability) {
				const T value = NumberGenerator<>::getInstance().get(MAX_RANGE);
				set(i, value);
			}
		}
	}

	bool operator==(const DNA& other) const
	{
		const uint64_t code_length = getBytesCount();
		if (other.getBytesCount() != code_length) {
			return false;
		}

		for (uint64_t i(0); i < code_length; ++i) {
			if (code[i] != other.code[i]) {
				return false;
			}
		}

		return true;
	}

	std::vector<byte> code;
};
