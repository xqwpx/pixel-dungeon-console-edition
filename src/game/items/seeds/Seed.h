#pragma once
#include "game/items/Item.h"
namespace pdv {
class Seed : public Item {
public:
    Seed() { stackable_ = true; name_ = "seed"; }
    std::unique_ptr<Item> clone() const override { return std::unique_ptr<Item>(new Seed(*this)); }
    bool isUpgradable() const override { return false; }
    bool isIdentified() const override { return true; }
};
}
