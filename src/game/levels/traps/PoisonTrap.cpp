#include "game/levels/traps/PoisonTrap.h"
#include "game/Dungeon.h"
#include "game/actors/Char.h"
#include "game/actors/buffs/Buff.h"
#include "game/actors/buffs/Poison.h"
namespace pdv {
void PoisonTrap::trigger(Dungeon& dungeon, int, Char* ch) {
    if (!ch) {
        return;
    }

    Poison* poison =
        Buff::affect<Poison>(*ch);

    if (poison) {
        poison->set(
            Poison::durationFactor(*ch) *
            static_cast<float>(
                4 + dungeon.depth() / 2
            )
        );
    }
}
}
