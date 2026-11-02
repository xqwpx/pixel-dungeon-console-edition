#include "game/items/potions/PotionOfMindVision.h"
#include "game/items/ItemSpriteSheet.h"
#include "game/Dungeon.h"
#include "game/actors/Hero.h"
#include "game/actors/buffs/Buff.h"
#include "game/actors/buffs/MindVision.h"

namespace pdv {
PotionOfMindVision::PotionOfMindVision() {
    image_ = ItemSpriteSheet::POTION_IVORY;
    name_ = "Potion of Mind Vision";
}
void PotionOfMindVision::apply(Dungeon& dungeon, Hero& hero) {
    setKnown();
    Buff::affect<MindVision>(
        hero,
        MindVision::Duration
    );
    dungeon.observe();
}
std::unique_ptr<Item> PotionOfMindVision::clone() const {
    return std::unique_ptr<Item>(new PotionOfMindVision(*this));
}
}
