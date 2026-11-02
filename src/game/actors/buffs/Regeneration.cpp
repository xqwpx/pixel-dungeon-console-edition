#include "game/actors/buffs/Regeneration.h"
#include "game/actors/Hero.h"
#include "game/items/rings/Ring.h"
#include <cmath>

namespace pdv {

bool Regeneration::act() {
    Hero* hero =
        dynamic_cast<Hero*>(target());

    if (!hero || !hero->isAlive()) {
        deactivate();
        next();
        return true;
    }

    if (hero->HP < hero->HT &&
        !hero->isStarving()) {
        hero->HP += 1;
    }

    int bonus=0;for(const Ring* ring:{hero->belongings().ring1(),hero->belongings().ring2()})if(ring)bonus+=ring->mendingBonus();
    spend(static_cast<float>(RegenerationDelay/std::pow(1.2,bonus)));
    next();

    return true;
}

} // namespace pdv
