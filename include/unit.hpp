#pragma once

#include "dna.hpp"

struct Unit
{
    Unit() = delete; // Explicitly delete the default constructor
    Unit(const uint64_t dna_bits_count)
        : dna(dna_bits_count)
        , fitness(0.0f)
        , alive(true)
    {}

    void loadDNA(const DNA& new_dna)
    {
        fitness = 0.0f;
        dna = new_dna;

        onUpdateDNA();
    }

    virtual void onUpdateDNA() = 0;

    DNA dna;
    float fitness;
    bool alive;
};