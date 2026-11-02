#pragma once
#include "game/actors/buffs/FlavourBuff.h"

namespace pdv {

class Invisibility : public FlavourBuff {
public:
    static constexpr float Duration = 15.0f;

    bool attachTo(Char& target) override;
    void detach() override;
};

} // namespace pdv
