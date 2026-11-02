#pragma once
#include "game/items/Item.h"

namespace pdv {
class Key : public Item {
public:
    static constexpr float TimeToUnlock = 1.0f;
    explicit Key(int depth=1) : depth_(depth) { stackable_=false; levelKnown_=true; cursedKnown_=true; }
    int depth() const { return depth_; }
    void depth(int d){ depth_=d; }
    bool isUpgradable() const override { return false; }
    bool isIdentified() const override { return true; }
private:
    int depth_=1;
};
} // namespace pdv
