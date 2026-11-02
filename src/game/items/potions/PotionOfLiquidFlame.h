#pragma once
#include "game/items/potions/Potion.h"

namespace pdv {
class PotionOfLiquidFlame : public Potion {
public:
    PotionOfLiquidFlame();
    void shatter(Dungeon& dungeon, int cell) override;
    std::unique_ptr<Item> clone() const override;
};
}
