#include "game/actors/buffs/Burning.h"

#include "game/levels/Level.h"
#include "game/utils/Random.h"
#include "game/actors/Hero.h"
#include "game/Progression.h"
#include "game/actors/buffs/Light.h"
#include "game/actors/blobs/Blob.h"
#include "game/items/scrolls/Scroll.h"
#include "game/items/ConsumableCatalog.h"
#include "game/items/SpecialItems.h"

namespace pdv {

bool Burning::act() {
    Char* ch = target();

    if (!ch || !ch->isAlive()) {
        detach();
        return true;
    }

    Level* level = ch->levelContext();

    const int damage=Random::Int(1,5);
    if(dynamic_cast<Hero*>(ch)&&damage>=ch->HP)Progression::instance().noteDeath(Badge::DeathFire);
    ch->damage(damage,this);

    if(Hero* hero=dynamic_cast<Hero*>(ch)){
        Buff::prolong<Light>(*hero,TICK*1.01f);
        const auto& items=hero->belongings().backpack().items();
        if(!items.empty()){
            Item* chosen=items[static_cast<std::size_t>(Random::Int(static_cast<int>(items.size())))].get();
            if(dynamic_cast<Scroll*>(chosen))hero->belongings().detach(chosen);
            else if(MysteryMeat* meat=dynamic_cast<MysteryMeat*>(chosen)){
                const int count=meat->quantity();auto removed=hero->belongings().detach(meat);
                if(removed){std::unique_ptr<PreparedFood> cooked(new PreparedFood(PreparedFoodKind::ChargrilledMeat));cooked->quantity(count);if(!hero->belongings().backpack().collect(std::move(cooked))&&level){std::unique_ptr<PreparedFood> dropped(new PreparedFood(PreparedFoodKind::ChargrilledMeat));dropped->quantity(count);level->drop(std::move(dropped),hero->pos);}}
            }
        }
    }

    spend(TICK);
    left_ -= TICK;

    bool extinguish =
        left_ <= 0.0f;

    if (!extinguish && ch->HT > 0) {
        const float threshold =
            (
                2.0f +
                static_cast<float>(ch->HP) /
                static_cast<float>(ch->HT)
            ) / 3.0f;

        extinguish =
            Random::Float() >
            threshold;
    }

    if(level&&level->flamable(ch->pos))level->seedBlob(BlobKind::Fire,ch->pos,4);

    if (!extinguish &&
        level &&
        level->water(ch->pos) &&
        !ch->flying) {
        extinguish = true;
    }

    if (extinguish) {
        detach();
    } else {
        next();
    }

    return true;
}

void Burning::reignite(Char&) {
    left_ = Duration;
}

} // namespace pdv
