#pragma once
#include "game/actors/buffs/Invisibility.h"

namespace pdv {

class Shadows : public Invisibility {
public:
    bool attachTo(Char& target) override;
    void detach() override;
    bool act() override;

    void prolong() { left_ = 2.0f; }
    float left() const { return left_; }
    float persistentValue() const override { return left_; }
    void restorePersistent(float cooldownValue,float value,int state) override { left_=value;Invisibility::restorePersistent(cooldownValue,value,state); }

private:
    float left_ = 0.0f;
};

} // namespace pdv
