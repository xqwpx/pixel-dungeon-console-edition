#include "game/actors/buffs/Hunger.h"

#include "game/actors/Hero.h"
#include "game/Progression.h"
#include "game/items/rings/Ring.h"
#include "game/utils/Random.h"
#include <algorithm>

namespace pdv {

void Hunger::bind(Hero* hero) {
    if (attached()) {
        detach();
    }

    if (hero) {
        attachTo(*hero);
    }
}

void Hunger::reset() {
    level_ = 0.0f;
    resetTime();
}

bool Hunger::act() {
    Hero* hero =
        dynamic_cast<Hero*>(target());

    if (!hero || !hero->isAlive()) {
        deactivate();
        next();
        return true;
    }

    if (isStarving()) {
        if (Random::Float() < 0.3f &&
            (hero->HP > 1 ||
             !hero->paralysed)) {

            if(hero->HP<=1)Progression::instance().noteDeath(Badge::DeathHunger);
            hero->damage(1,this);
        }
    } else {
        int bonus=0;for(const Ring* ring:{hero->belongings().ring1(),hero->belongings().ring2()})if(ring)bonus+=ring->satietyBonus();
        const float enduranceFactor=std::max(0.35f,1.0f-static_cast<float>(hero->enduranceUpgrades())*0.08f);
        level_ += std::max(0.0f,(Step-static_cast<float>(bonus))*enduranceFactor);
    }

    const float step =
        hero->heroClass() ==
            HeroClass::Rogue
        ? Step * 1.2f
        : Step;

    spend(step);
    next();

    return true;
}

void Hunger::satisfy(float energy) {
    level_ -= energy;

    if (level_ < 0.0f) {
        level_ = 0.0f;
    } else if (level_ > Starving) {
        level_ = Starving;
    }
}

} // namespace pdv
