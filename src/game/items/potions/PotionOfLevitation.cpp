#include "game/items/potions/PotionOfLevitation.h"
#include "game/items/ItemSpriteSheet.h"
#include "game/actors/Hero.h"
#include "game/actors/buffs/Buff.h"
#include "game/actors/buffs/Levitation.h"

namespace pdv {
PotionOfLevitation::PotionOfLevitation() {
    image_ = ItemSpriteSheet::POTION_MAGENTA;
    name_ = "Potion of Levitation";
}
void PotionOfLevitation::apply(Dungeon&, Hero& hero) {
    setKnown();
    Buff::affect<Levitation>(
        hero,
        Levitation::Duration
    );
}
std::unique_ptr<Item> PotionOfLevitation::clone() const {
    return std::unique_ptr<Item>(new PotionOfLevitation(*this));
}
}
