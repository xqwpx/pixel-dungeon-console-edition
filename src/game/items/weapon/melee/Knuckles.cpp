#include "game/items/weapon/melee/Knuckles.h"
#include "game/items/ItemSpriteSheet.h"

namespace pdv {

Knuckles::Knuckles()
    : MeleeWeapon(1, 1.0f, 0.5f) {

    name_ = "knuckleduster";
    image_ = ItemSpriteSheet::KNUCKLEDUSTER;
}

std::unique_ptr<Item> Knuckles::clone() const {
    return std::unique_ptr<Item>(new Knuckles(*this));
}

} // namespace pdv
