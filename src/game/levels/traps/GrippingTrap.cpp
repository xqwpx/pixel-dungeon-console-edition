#include "game/levels/traps/GrippingTrap.h"
#include "game/Dungeon.h"
#include "game/actors/Char.h"
#include "game/actors/buffs/Bleeding.h"
#include "game/actors/buffs/Buff.h"
#include "game/actors/buffs/Cripple.h"
#include "game/utils/Random.h"
#include <algorithm>

namespace pdv {
void GrippingTrap::trigger(Dungeon& dungeon, int, Char* ch) {
    if (!ch) {
        return;
    }

    const int damage =
        std::max(
            0,
            (dungeon.depth() + 3) -
            Random::IntRange(
                0,
                ch->dr() / 2
            )
        );

    Bleeding* bleeding =
        Buff::affect<Bleeding>(*ch);

    if (bleeding) {
        bleeding->set(damage);
    }

    Buff::prolong<Cripple>(
        *ch,
        Cripple::Duration
    );

    // Wound visual is presentation-only.
}
}
