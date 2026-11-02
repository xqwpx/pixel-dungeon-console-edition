#pragma once
#include "game/items/keys/Key.h"
#include "game/items/ItemSpriteSheet.h"
namespace pdv {
class IronKey : public Key {
public:
    explicit IronKey(int depth=1) : Key(depth) {name_="iron key";image_=ItemSpriteSheet::IRON_KEY;}
    std::unique_ptr<Item> clone() const override { return std::unique_ptr<Item>(new IronKey(*this)); }
};
} // namespace pdv
