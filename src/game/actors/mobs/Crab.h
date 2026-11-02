#pragma once

#include "game/actors/mobs/Mob.h"

namespace pdv {

class Crab : public Mob {
public:
    Crab();
    int damageRoll() const override;
    int attackSkill(const Char& target) const override;
    int dr() const override;
    const char* defenseVerb() const override { return "parried"; }
    const char* description() const override;
protected:float lootChance()const override{return 0.167f;}std::unique_ptr<Item>createLoot()const override;
};

} // namespace pdv
