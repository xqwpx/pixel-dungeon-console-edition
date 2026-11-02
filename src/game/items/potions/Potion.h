#pragma once
#include <string>
#include <cstdint>
#include "game/items/Item.h"

namespace pdv {

class Dungeon;
class Hero;
class ItemStatusHandler;

class Potion : public Item {
public:
    static constexpr float TimeToDrink = 1.0f;
    static constexpr int Count = 12;

    Potion();

    static void initColors();

    bool isKnown() const;
    void setKnown();
    Item& identify() override;
    int price() const override;

    bool isIdentified() const override { return isKnown(); }
    bool isUpgradable() const override { return false; }

    int image() const override;
    std::string displayName() const override;
    std::string color() const;
    std::string description() const;

    virtual void apply(Dungeon& dungeon, Hero& hero);
    virtual void shatter(Dungeon& dungeon, int cell);

    void onThrow(Dungeon& dungeon, Hero& hero, int cell);
    std::vector<ItemAction> actions(const Hero&) const override { return {ItemAction::Drink,ItemAction::Throw,ItemAction::Drop}; }

    static int statusIndex(const Potion& potion);
    static uint16_t knownMask();
    static void restoreKnownMask(uint16_t mask);

private:
    static ItemStatusHandler& handler();
};

} // namespace pdv
