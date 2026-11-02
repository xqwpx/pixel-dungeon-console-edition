#pragma once
#include "game/items/Item.h"

namespace pdv {
class Hero;
class Food : public Item {
public:
    static constexpr float TimeToEat = 3.0f;
    Food();
    float energy() const { return energy_; }
    void setEnergy(float value) { energy_ = value; }
    bool eat(Hero& hero);
    std::unique_ptr<Item> clone() const override;
    bool isUpgradable() const override { return false; }
    bool isIdentified() const override { return true; }
    int price() const override { return 10*quantity(); }
    std::vector<ItemAction> actions(const Hero&) const override { return {ItemAction::Eat,ItemAction::Drop}; }
private:
    float energy_ = 260.0f;
};
} // namespace pdv
