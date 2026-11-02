#include "game/levels/traps/AlarmTrap.h"
#include "game/Dungeon.h"
#include "game/actors/Char.h"
#include "game/actors/mobs/Mob.h"
namespace pdv {
void AlarmTrap::trigger(Dungeon& dungeon, int pos, Char* ch) {
    for (auto& mob : dungeon.level().mobsMutable()) {
        if (!mob ||
            !mob->isAlive() ||
            mob.get() == ch) {
            continue;
        }

        mob->beckon(pos);
    }

    // Sound/log/particles are Vita presentation-layer work.
}
}
