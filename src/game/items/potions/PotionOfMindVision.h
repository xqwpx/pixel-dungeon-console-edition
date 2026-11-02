#pragma once
#include "game/items/potions/Potion.h"

namespace pdv {
class PotionOfMindVision : public Potion {
public:
    PotionOfMindVision();
    void apply(Dungeon& dungeon, Hero& hero) override;
    std::unique_ptr<Item> clone() const override;
};
}
