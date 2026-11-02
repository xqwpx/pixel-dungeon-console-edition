#pragma once
#include "game/actors/buffs/FlavourBuff.h"

namespace pdv {

class Cripple : public FlavourBuff {
public:
    static constexpr float Duration = 10.0f;
};

} // namespace pdv
