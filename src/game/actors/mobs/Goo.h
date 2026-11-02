#pragma once

#include "game/actors/mobs/Mob.h"

namespace pdv {

class Goo : public Mob {
public:
    Goo();
    bool act() override;
    int damageRoll() const override;
    int attackSkill(const Char& target) const override;
    int attackProc(Char& enemy, int damage) override;
    int dr() const override;
    void die(const void* cause) override;
    bool immuneTo(const std::type_info&) const override;
    bool resistantTo(const std::type_info&) const override;
    const char* description() const override;

    bool pumpedUp() const { return pumpedUp_; }
    void restorePumpedUp(bool value) { pumpedUp_ = value; }
    bool isBoss()const override{return true;}

protected:
    float lootChance()const override{return 0.333f;}
    std::unique_ptr<Item>createLoot()const override;
    bool canAttack(const Char& enemy) const override;
    bool doAttack(Char& enemy) override;
    bool getCloser(int target);

private:
    bool pumpedUp_ = false;
    bool jumped_ = false;
};

} // namespace pdv
