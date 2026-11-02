#include "game/items/potions/PotionOfInvisibility.h"
#include "game/items/ItemSpriteSheet.h"
#include "game/actors/Hero.h"
#include "game/actors/buffs/Buff.h"
#include "game/actors/buffs/Invisibility.h"

namespace pdv {
PotionOfInvisibility::PotionOfInvisibility() {
    image_ = ItemSpriteSheet::POTION_BISTRE;
    name_ = "Potion of Invisibility";
}
void PotionOfInvisibility::apply(Dungeon&, Hero& hero) {
    setKnown();
    Buff::affect<Invisibility>(
        hero,
        Invisibility::Duration
    );
}
std::unique_ptr<Item> PotionOfInvisibility::clone() const {
    return std::unique_ptr<Item>(new PotionOfInvisibility(*this));
}
}
