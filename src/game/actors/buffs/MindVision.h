#pragma once
#include "game/actors/buffs/FlavourBuff.h"

namespace pdv {

class MindVision : public FlavourBuff {
public:
    static constexpr float Duration = 20.0f;
    int distance = 2;
};

} // namespace pdv
