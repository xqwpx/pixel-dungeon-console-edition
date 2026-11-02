#pragma once
#include "game/actors/buffs/FlavourBuff.h"

namespace pdv {

class Sleep : public FlavourBuff {
public:
    static constexpr float SWS = 1.5f;
};

} // namespace pdv
