#pragma once
#include "game/items/potions/Potion.h"

namespace pdv {
class PotionOfToxicGas : public Potion {
public:
    PotionOfToxicGas();
    void shatter(Dungeon& dungeon, int cell) override;
    std::unique_ptr<Item> clone() const override;
};
}
