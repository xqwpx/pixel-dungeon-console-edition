#pragma once
#include "game/items/keys/Key.h"
#include "game/items/ItemSpriteSheet.h"
namespace pdv {
class GoldenKey : public Key {
public:
    explicit GoldenKey(int depth=1) : Key(depth) {name_="golden key";image_=ItemSpriteSheet::GOLDEN_KEY;}
    std::unique_ptr<Item> clone() const override { return std::unique_ptr<Item>(new GoldenKey(*this)); }
};
} // namespace pdv
