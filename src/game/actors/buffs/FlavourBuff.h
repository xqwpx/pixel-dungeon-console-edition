#pragma once
#include "game/actors/buffs/Buff.h"

namespace pdv {

class FlavourBuff : public Buff {
public:
    bool act() override;
};

} // namespace pdv
