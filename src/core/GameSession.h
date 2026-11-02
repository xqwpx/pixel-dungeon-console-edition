#pragma once

#include <cstdint>

namespace pdv {

enum class HeroClass : int {
    Warrior = 0,
    Mage = 1,
    Rogue = 2,
    Huntress = 3
};

const char* heroClassName(HeroClass heroClass);

struct GameSession {
    HeroClass selectedHero = HeroClass::Warrior;

    bool newGameRequested = false;
    bool continueRequested = false;

    // Incremented every time StartScene launches a fresh dungeon.
    // Seed used by the native level generator; incremented for each new run.
    uint32_t runSeed = 0;
};

} // namespace pdv
