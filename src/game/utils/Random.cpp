#include "game/utils/Random.h"

#include <algorithm>

namespace pdv {

std::mt19937 Random::rng_{0x50445636u};

void Random::seed(uint32_t seed) {
    rng_.seed(
        seed ? seed : 0x50445636u
    );
}

float Random::Float() {
    return std::generate_canonical<float, 24>(rng_);
}

float Random::Float(float max) {
    return Float() * max;
}

float Random::Float(float min, float max) {
    return min + Float() * (max - min);
}

int Random::Int(int max) {
    if (max <= 0) {
        return 0;
    }

    std::uniform_int_distribution<int> d(
        0,
        max - 1
    );

    return d(rng_);
}

int Random::Int(int min, int max) {
    if (max <= min) {
        return min;
    }

    std::uniform_int_distribution<int> d(
        min,
        max - 1
    );

    return d(rng_);
}

int Random::IntRange(int min, int max) {
    if (max <= min) {
        return min;
    }

    std::uniform_int_distribution<int> d(
        min,
        max
    );

    return d(rng_);
}

int Random::NormalIntRange(int min, int max) {
    // PD-classes Random.NormalIntRange:
    // average of two inclusive uniform IntRange rolls.
    return (
        IntRange(min, max) +
        IntRange(min, max)
    ) / 2;
}

} // namespace pdv
