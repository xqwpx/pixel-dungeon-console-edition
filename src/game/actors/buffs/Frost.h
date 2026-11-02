#pragma once
#include "game/actors/buffs/FlavourBuff.h"
#include "game/items/rings/Ring.h"

namespace pdv {

class Frost : public FlavourBuff {
public:
    static constexpr float Duration = 5.0f;

    bool attachTo(Char& target) override;
    void detach() override;

    static float duration(const Char& ch) { return Duration*elementsDurationFactor(ch); }
};

} // namespace pdv
