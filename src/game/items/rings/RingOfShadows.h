#pragma once
#include "game/items/rings/Ring.h"
#include "game/items/ItemSpriteSheet.h"
namespace pdv {
class RingOfShadows: public Ring {
public:
    RingOfShadows(){name_="Ring of Shadows";image_=39;}
    int statusIndex() const override { return 11; }
    int stealthBonus() const override { return effectiveLevel(); }
    std::unique_ptr<Item> clone() const override { return std::unique_ptr<Item>(new RingOfShadows(*this)); }
};
} // namespace pdv
