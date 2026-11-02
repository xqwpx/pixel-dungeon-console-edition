#pragma once
#include "game/actors/buffs/FlavourBuff.h"

namespace pdv {

class Levitation : public FlavourBuff {
public:
    static constexpr float Duration = 20.0f;

    bool attachTo(Char& target) override;
    void detach() override;
};

} // namespace pdv
