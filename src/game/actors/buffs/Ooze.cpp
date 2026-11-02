#include "game/actors/buffs/Ooze.h"
#include "game/levels/Level.h"

namespace pdv {

bool Ooze::act() {
    Char* ch = target();

    if (!ch || !ch->isAlive()) {
        detach();
        return true;
    }

    ch->damage(damage, this);
    spend(TICK);

    Level* level = ch->levelContext();

    if (level &&
        level->water(ch->pos)) {
        detach();
    } else {
        next();
    }

    return true;
}

} // namespace pdv
