#pragma once
#include "game/items/Item.h"
namespace pdv {
class Gold : public Item {
public:
    explicit Gold(int value = 1);
    Gold& random(int depth);
    std::unique_ptr<Item> clone() const override;
    bool isUpgradable() const override { return false; }
    bool isIdentified() const override { return true; }
};
}
