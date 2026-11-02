#pragma once
#include "game/actors/buffs/Buff.h"

namespace pdv {

class Burning : public Buff {
public:
    static constexpr float Duration = 8.0f;

    bool act() override;

    void reignite(Char& target);
    float left() const { return left_; }
    float persistentValue() const override { return left_; }
    void restorePersistent(float cooldownValue,float value,int state) override { left_=value;Buff::restorePersistent(cooldownValue,value,state); }

private:
    float left_ = Duration;
};

} // namespace pdv
