#pragma once

#include "game/actors/mobs/Mob.h"

namespace pdv {

class Rat : public Mob {
public:
    Rat();

    int damageRoll() const override;
    int attackSkill(const Char& target) const override;
    int dr() const override;

    const char* description() const override;
};

} // namespace pdv
