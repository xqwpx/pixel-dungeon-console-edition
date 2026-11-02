#pragma once

#include <vector>

namespace pdv {

class Char;

class BuffIndicator {
public:
    static constexpr int None = -1;

    static constexpr int MindVision = 0;
    static constexpr int Levitation = 1;
    static constexpr int Fire = 2;
    static constexpr int Poison = 3;
    static constexpr int Paralysis = 4;
    static constexpr int Hunger = 5;
    static constexpr int Starvation = 6;
    static constexpr int Slow = 7;
    static constexpr int Ooze = 8;
    static constexpr int Amok = 9;
    static constexpr int Terror = 10;
    static constexpr int Roots = 11;
    static constexpr int Invisible = 12;
    static constexpr int Shadows = 13;
    static constexpr int Weakness = 14;
    static constexpr int Frost = 15;
    static constexpr int Blindness = 16;
    static constexpr int Combo = 17;
    static constexpr int Fury = 18;
    static constexpr int Healing = 19;
    static constexpr int Armor = 20;
    static constexpr int Heart = 21;
    static constexpr int Light = 22;
    static constexpr int Cripple = 23;
    static constexpr int Barkskin = 24;
    static constexpr int Immunity = 25;
    static constexpr int Bleeding = 26;
    static constexpr int Mark = 27;
    static constexpr int Deferred = 28;
    static constexpr int Vertigo = 29;
    static constexpr int Rage = 30;
    static constexpr int Sacrifice = 31;

    static constexpr int Size = 7;
    static constexpr int AtlasColumns = 18;

    static std::vector<int> iconsFor(
        const Char& ch
    );
};

} // namespace pdv
