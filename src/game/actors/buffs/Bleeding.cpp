#include "game/actors/buffs/Bleeding.h"
#include "game/utils/Random.h"

namespace pdv {

bool Bleeding::act() {
    Char* ch = target();

    if (!ch || !ch->isAlive()) {
        detach();
        return true;
    }

    level_ =
        Random::Int(
            level_ / 2,
            level_
        );

    if (level_ > 0) {
        ch->damage(
            level_,
            this
        );

        spend(TICK);
        next();
    } else {
        detach();
    }

    return true;
}

} // namespace pdv
