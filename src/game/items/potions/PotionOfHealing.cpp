#include "game/items/potions/PotionOfHealing.h"
#include "game/items/ItemSpriteSheet.h"

#include "game/actors/Hero.h"
#include "game/actors/buffs/Bleeding.h"
#include "game/actors/buffs/Buff.h"
#include "game/actors/buffs/Cripple.h"
#include "game/actors/buffs/Poison.h"
#include "game/actors/buffs/Weakness.h"

namespace pdv {

PotionOfHealing::PotionOfHealing() {
    image_ = ItemSpriteSheet::POTION_TURQUOISE;
    name_ = "Potion of Healing";
}

void PotionOfHealing::apply(
    Dungeon&,
    Hero& hero
) {
    setKnown();
    heal(hero);
}

void PotionOfHealing::heal(
    Hero& hero
) {
    hero.HP = hero.HT;

    Buff::detach<Poison>(hero);
    Buff::detach<Cripple>(hero);
    Buff::detach<Weakness>(hero);
    Buff::detach<Bleeding>(hero);
}

std::unique_ptr<Item>
PotionOfHealing::clone() const {
    return std::unique_ptr<Item>(
        new PotionOfHealing(*this)
    );
}

} // namespace pdv
