#pragma once
#include "game/items/weapon/Weapon.h"
namespace pdv {
class Dungeon;
class MissileWeapon: public Weapon {
public:
    MissileWeapon();
    bool isMissileWeapon() const override { return true; }
    bool isUpgradable() const override { return false; }
    virtual bool returnsToOwner() const { return false; }
    virtual bool onMiss(Dungeon&,int) { return false; }
};
} // namespace pdv
