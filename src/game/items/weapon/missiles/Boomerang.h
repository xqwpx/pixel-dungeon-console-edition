#pragma once
#include "game/items/weapon/missiles/MissileWeapon.h"
namespace pdv {
class Boomerang: public MissileWeapon {
public:
    Boomerang();
    int min() const override { return 1 + level_; }
    int max() const override { return 4 + 2*level_; }
    bool isUpgradable() const override { return true; }
    bool returnsToOwner() const override { return true; }
    std::unique_ptr<Item> clone() const override;
};
} // namespace pdv
