#include "game/items/weapon/melee/ShortSword.h"
#include "game/items/ItemSpriteSheet.h"

namespace pdv {

ShortSword::ShortSword()
    : MeleeWeapon(1, 1.0f, 1.0f) {

    name_ = "short sword";
    image_ = ItemSpriteSheet::SHORT_SWORD;
    strength_ = 11;
}

int ShortSword::max0() const {
    return 12;
}

std::unique_ptr<Item> ShortSword::clone() const {
    return std::unique_ptr<Item>(new ShortSword(*this));
}

} // namespace pdv
