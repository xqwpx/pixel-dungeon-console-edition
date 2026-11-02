#pragma once
#include "game/items/potions/Potion.h"

namespace pdv {
class PotionOfParalyticGas : public Potion {
public:
    PotionOfParalyticGas();
    void shatter(Dungeon& dungeon, int cell) override;
    std::unique_ptr<Item> clone() const override;
};
}
