#pragma once
#include "game/items/potions/Potion.h"

namespace pdv {

class PotionOfHealing : public Potion {
public:
    PotionOfHealing();

    void apply(
        Dungeon& dungeon,
        Hero& hero
    ) override;

    static void heal(Hero& hero);

    std::unique_ptr<Item> clone() const override;
};

} // namespace pdv
