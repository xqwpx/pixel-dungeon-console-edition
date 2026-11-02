#pragma once
#include "game/actors/buffs/Buff.h"

namespace pdv {

class Ooze : public Buff {
public:
    int damage = 1;
    bool act() override;
};

} // namespace pdv
