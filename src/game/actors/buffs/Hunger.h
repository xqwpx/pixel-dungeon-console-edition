#pragma once
#include "game/actors/buffs/Buff.h"

namespace pdv {

class Hero;

class Hunger : public Buff {
public:
    static constexpr float Step = 10.0f;
    static constexpr float Hungry = 260.0f;
    static constexpr float Starving = 360.0f;

    void bind(Hero* hero);
    void reset();

    bool act() override;

    void satisfy(float energy);

    float level() const { return level_; }
    void restoreLevel(float value) { level_ = value < 0.0f ? 0.0f : (value > Starving ? Starving : value); }
    bool isHungry() const { return level_ >= Hungry; }
    bool isStarving() const { return level_ >= Starving; }

private:
    float level_ = 0.0f;
};

} // namespace pdv
