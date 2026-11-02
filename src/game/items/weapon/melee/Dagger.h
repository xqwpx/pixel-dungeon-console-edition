#pragma once
#include "game/items/weapon/melee/MeleeWeapon.h"

namespace pdv {

class Dagger : public MeleeWeapon {
public:
    Dagger();
    std::unique_ptr<Item> clone() const override;
};

} // namespace pdv
