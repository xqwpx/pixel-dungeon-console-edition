#pragma once
#include "game/items/keys/Key.h"
#include "game/items/ItemSpriteSheet.h"
namespace pdv {
class SkeletonKey : public Key {
public:
    explicit SkeletonKey(int depth=1) : Key(depth) {name_="skeleton key";image_=ItemSpriteSheet::SKELETON_KEY;}
    std::unique_ptr<Item> clone() const override { return std::unique_ptr<Item>(new SkeletonKey(*this)); }
};
} // namespace pdv
