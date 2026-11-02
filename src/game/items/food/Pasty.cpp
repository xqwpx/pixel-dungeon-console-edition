#include "game/items/food/Pasty.h"
#include "game/items/ItemSpriteSheet.h"
#include "game/actors/buffs/Hunger.h"
namespace pdv {
Pasty::Pasty() : Food() {
    name_ = "pasty";
    image_ = ItemSpriteSheet::PASTY;
    setEnergy(Hunger::Starving);
}
std::unique_ptr<Item> Pasty::clone() const {
    return std::unique_ptr<Item>(new Pasty(*this));
}
}
