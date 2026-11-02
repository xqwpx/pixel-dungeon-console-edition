#pragma once
#include "game/items/weapon/Weapon.h"

namespace pdv {

class MeleeWeapon : public Weapon {
public:
    MeleeWeapon(int tier, float accuracy, float delay);

    int min() const override;
    int max() const override;

    int tier() const { return tier_; }
    int typicalSTR() const;
    int price() const override;

protected:
    virtual int min0() const;
    virtual int max0() const;

private:
    int tier_ = 1;
};

} // namespace pdv
