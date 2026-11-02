#pragma once
#include "game/actors/buffs/Buff.h"

namespace pdv {

class Bleeding : public Buff {
public:
    void set(int level) { level_ = level; }
    int level() const { return level_; }
    int persistentInt() const override { return level_; }
    void restorePersistent(float cooldownValue,float value,int state) override { level_=state;Buff::restorePersistent(cooldownValue,value,state); }

    bool act() override;

private:
    int level_ = 0;
};

} // namespace pdv
