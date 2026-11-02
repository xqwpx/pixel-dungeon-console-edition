#pragma once
#include "game/actors/buffs/Buff.h"
namespace pdv {
class DeferredDamage : public Buff {
public:
    bool attachTo(Char& target) override;
    bool act() override;
    void prolong(int damage) { if (damage > 0) damage_ += damage; }
    int remainingDamage() const { return damage_; }
    int persistentInt() const override { return damage_; }
    void restorePersistent(float cooldownValue,float value,int state) override;
private:int damage_=0;
};
}
