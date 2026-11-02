#pragma once
#include "game/items/weapon/melee/MeleeWeapon.h"

namespace pdv {

class Knuckles : public MeleeWeapon {
public:
    Knuckles();
    std::unique_ptr<Item> clone() const override;
};

} // namespace pdv
