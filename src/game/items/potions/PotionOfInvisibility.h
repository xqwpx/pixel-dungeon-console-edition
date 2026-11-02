#pragma once
#include "game/items/potions/Potion.h"

namespace pdv {
class PotionOfInvisibility : public Potion {
public:
    PotionOfInvisibility();
    void apply(Dungeon& dungeon, Hero& hero) override;
    std::unique_ptr<Item> clone() const override;
};
}
