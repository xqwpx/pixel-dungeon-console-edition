#include "game/items/armor/ClothArmor.h"
#include "game/items/ItemSpriteSheet.h"

namespace pdv {

ClothArmor::ClothArmor()
    : Armor(1) {

    name_ = "cloth armor";
    image_ = ItemSpriteSheet::ARMOR_CLOTH;
}

std::unique_ptr<Item> ClothArmor::clone() const {
    return std::unique_ptr<Item>(new ClothArmor(*this));
}

} // namespace pdv
