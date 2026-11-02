#pragma once

#include "game/actors/mobs/Mob.h"

namespace pdv {

class Swarm : public Mob {
public:
    Swarm();
    int damageRoll() const override;
    int attackSkill(const Char& target) const override;
    int defenseProc(Char& enemy, int damage) override;
    const char* description() const override;
    int persistentState() const override { return generation_; }
    void restorePersistentState(int value) override { generation_ = value < 0 ? 0 : value; }

private:
    int generation_ = 0;
    float lootChance() const override;
    std::unique_ptr<Item> createLoot() const override;
};

} // namespace pdv
