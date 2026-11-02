#pragma once
#include "game/items/weapon/missiles/MissileWeapon.h"
namespace pdv {
class Dart: public MissileWeapon {
public:
    explicit Dart(int number=1);
    int min() const override { return 1; }
    int max() const override { return 4; }
    int price() const override { return 2*quantity(); }
    std::unique_ptr<Item> clone() const override;
};
} // namespace pdv
