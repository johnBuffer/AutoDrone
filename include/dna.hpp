#pragma once

#include <vector>
#include <iostream>
#include "number_generator.hpp"
#include <cstring>
#include "utils.hpp"

constexpr float MAX_RANGE = 10.0f;

struct DNA
{
    using byte = uint8_t;

    // Constructor initializing `code` with the size needed for `bits_count` bits.
    DNA(const uint64_t bits_count)
        : code(bits_count / 8 + (bits_count % 8 != 0 ? 1 : 0)) // Correctly handle bits not aligned to byte boundaries
    {}

    template<typename T>
    void initialize(const float range)
    {
        const uint64_t element_count = getElementsCount<T>();
        for (uint64_t i = 0; i < element_count; ++i) {
            const T value = NumberGenerator<>::getInstance().get(range);
            set(i, value);
        }
    }

    template<typename T>
    T get(const uint64_t offset) const
    {
        T result;
        const uint64_t dna_offset = offset * sizeof(T);
        std::memcpy(&result, &code[dna_offset], sizeof(T));
        return result;
    }

    template<typename T>
    void set(const uint64_t offset, const T& value)
    {
        const T checked_value = clamp(-MAX_RANGE, MAX_RANGE, value); // Ensure value is within range
        const uint64_t dna_offset = offset * sizeof(T);
        std::memcpy(&code[dna_offset], &checked_value, sizeof(T));
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

    void mutateBits(const float probability)
    {
        for (byte& b : code) {
            for (uint64_t i = 0; i < 8; ++i) {
                if (NumberGenerator<>::getInstance().getUnder(1.0f) < probability) {
                    const uint8_t mask = 1 << i; // Use bit shift for clarity
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
        for (uint64_t i = 0; i < element_count; ++i) {
            if (NumberGenerator<>::getInstance().getUnder(1.0f) < probability) {
                const T value = NumberGenerator<>::getInstance().get(MAX_RANGE);
                set(i, value);
            }
        }
    }

    bool operator==(const DNA& other) const
    {
        if (other.getBytesCount() != getBytesCount()) {
            return false;
        }

        return std::equal(code.begin(), code.end(), other.code.begin());
    }

    std::vector<byte> code;
};