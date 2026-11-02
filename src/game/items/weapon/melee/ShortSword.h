#pragma once
#include "game/items/weapon/melee/MeleeWeapon.h"

namespace pdv {

class ShortSword : public MeleeWeapon {
public:
    ShortSword();
    std::unique_ptr<Item> clone() const override;

protected:
    int max0() const override;
};

} // namespace pdv
