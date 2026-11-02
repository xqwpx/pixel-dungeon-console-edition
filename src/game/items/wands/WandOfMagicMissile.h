#pragma once
#include "game/items/wands/Wand.h"
namespace pdv {
class WandOfMagicMissile: public Wand {
public:
    WandOfMagicMissile();
    int initialCharges() const override { return 3; }
    void onZap(Dungeon& dungeon, Hero& hero, int cell) override;
    std::unique_ptr<Item> clone() const override;
};
} // namespace pdv
