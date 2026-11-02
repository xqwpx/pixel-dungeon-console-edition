#include "game/items/weapon/melee/Dagger.h"
#include "game/items/ItemSpriteSheet.h"

namespace pdv {

Dagger::Dagger()
    : MeleeWeapon(1, 1.2f, 1.0f) {

    name_ = "dagger";
    image_ = ItemSpriteSheet::DAGGER;
}

std::unique_ptr<Item> Dagger::clone() const {
    return std::unique_ptr<Item>(new Dagger(*this));
}

} // namespace pdv
