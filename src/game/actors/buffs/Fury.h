#pragma once
#include "game/actors/buffs/Buff.h"

namespace pdv {
class Fury : public Buff {
public:
    static constexpr float Level = 0.4f;
    bool act() override;
};
}
