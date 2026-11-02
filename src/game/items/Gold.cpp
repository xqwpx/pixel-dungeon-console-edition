#include "game/items/Gold.h"
#include "game/items/ItemSpriteSheet.h"
#include "game/utils/Random.h"
namespace pdv {
Gold::Gold(int value) {
    name_ = "gold";
    image_ = ItemSpriteSheet::GOLD;
    stackable_ = true;
    quantity_ = value;
    levelKnown_ = cursedKnown_ = true;
}
Gold& Gold::random(int depth) {
    quantity_ = Random::Int(
        20 + depth * 10,
        40 + depth * 20
    );
    return *this;
}
std::unique_ptr<Item> Gold::clone() const {
    return std::unique_ptr<Item>(new Gold(*this));
}
}
