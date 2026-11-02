#pragma once

#include "game/actors/mobs/Mob.h"

namespace pdv {

class Gnoll : public Mob {
public:
    Gnoll();
    int damageRoll() const override;
    int attackSkill(const Char& target) const override;
    int dr() const override;
    const char* description() const override;
protected:float lootChance()const override{return 0.5f;}std::unique_ptr<Item>createLoot()const override;
};

} // namespace pdv
