#pragma once
#include "game/actors/buffs/FlavourBuff.h"

namespace pdv {

class Weakness : public FlavourBuff {
public:
    static constexpr float Duration = 40.0f;

    bool attachTo(Char& target) override;
    void detach() override;
};

} // namespace pdv
