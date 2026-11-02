#pragma once
#include "game/actors/buffs/Buff.h"
#include "game/items/rings/Ring.h"

namespace pdv {

class Poison : public Buff {
public:
    void set(float duration) { left_ = duration; }
    static float durationFactor(const Char& ch) { return elementsDurationFactor(ch); }
    float left() const { return left_; }
    float persistentValue() const override { return left_; }
    void restorePersistent(float cooldownValue,float value,int state) override { left_=value;Buff::restorePersistent(cooldownValue,value,state); }

    bool act() override;

private:
    float left_ = 0.0f;
};

} // namespace pdv
