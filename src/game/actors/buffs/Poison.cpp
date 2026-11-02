#include "game/actors/buffs/Poison.h"
#include "game/actors/Hero.h"
#include "game/Progression.h"
#include <algorithm>

namespace pdv {

bool Poison::act() {
    Char* ch = target();

    if (!ch || !ch->isAlive()) {
        detach();
        return true;
    }

    int damage=static_cast<int>(left_/3.0f)+1;
    if(ch->resistantTo(typeid(Poison))) damage=std::max(1,damage/2);
    if(dynamic_cast<Hero*>(ch)&&damage>=ch->HP)Progression::instance().noteDeath(Badge::DeathPoison);
    ch->damage(damage,this);

    spend(TICK);

    left_ -= TICK;

    if (left_ <= 0.0f) {
        detach();
    } else {
        next();
    }

    return true;
}

} // namespace pdv
