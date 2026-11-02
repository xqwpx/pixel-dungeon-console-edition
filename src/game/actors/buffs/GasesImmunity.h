#pragma once
#include "game/actors/buffs/FlavourBuff.h"

namespace pdv {

class GasesImmunity : public FlavourBuff {
public:
    static constexpr float Duration = 5.0f;
};

} // namespace pdv
