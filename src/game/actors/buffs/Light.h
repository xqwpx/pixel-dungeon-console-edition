#pragma once
#include "game/actors/buffs/FlavourBuff.h"

namespace pdv {

class Light : public FlavourBuff {
public:
    static constexpr float Duration = 250.0f;
    static constexpr int Distance = 4;

    bool attachTo(Char& target) override;
    void detach() override;
};

} // namespace pdv
