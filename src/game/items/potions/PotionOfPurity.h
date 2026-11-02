#pragma once
#include "game/items/potions/Potion.h"

namespace pdv {
class PotionOfPurity : public Potion {
public:
    PotionOfPurity();
    void apply(Dungeon& dungeon, Hero& hero) override;
    void shatter(Dungeon& dungeon, int cell) override;
    std::unique_ptr<Item> clone() const override;
};
}
