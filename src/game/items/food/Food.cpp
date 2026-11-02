#include "game/items/food/Food.h"
#include "game/actors/Hero.h"
#include "game/actors/buffs/Hunger.h"
#include "game/items/ItemSpriteSheet.h"
#include <algorithm>

namespace pdv {
Food::Food(){ stackable_=true; name_="ration of food"; image_=ItemSpriteSheet::RATION; levelKnown_=true; cursedKnown_=true; }

bool Food::eat(Hero& hero){
    Hunger* hunger=hero.hunger();
    if (!hunger) return false;
    hunger->satisfy(energy_);

    switch(hero.heroClass()){
        case HeroClass::Warrior:
            if (hero.HP < hero.HT) hero.HP=std::min(hero.HP+5, hero.HT);
            break;
        case HeroClass::Mage:
            // Wand charge + ScrollOfRecharging side effects wait for Wand subsystem.
            break;
        case HeroClass::Rogue:
        case HeroClass::Huntress:
            break;
    }

    hero.spendAndNext(TimeToEat);
    return true;
}

std::unique_ptr<Item> Food::clone() const { return std::unique_ptr<Item>(new Food(*this)); }
} // namespace pdv
