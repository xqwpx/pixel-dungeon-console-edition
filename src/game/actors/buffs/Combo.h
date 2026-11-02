#pragma once
#include "game/actors/buffs/Buff.h"

namespace pdv {
class Combo : public Buff {
public:
    int count() const { return count_; }
    int persistentInt() const override { return count_; }
    void restorePersistent(float cooldownValue,float value,int state) override { count_=state;Buff::restorePersistent(cooldownValue,value,state); }
    int hit(int damage);
    bool act() override;
private:
    int count_ = 0;
};
}
