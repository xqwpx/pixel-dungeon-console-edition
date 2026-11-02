#pragma once
#include "game/items/potions/Potion.h"

namespace pdv {
class PotionOfLevitation : public Potion {
public:
    PotionOfLevitation();
    void apply(Dungeon& dungeon, Hero& hero) override;
    std::unique_ptr<Item> clone() const override;
};
}
