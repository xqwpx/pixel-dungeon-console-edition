#include "game/actors/blobs/ParalyticGas.h"

#include "game/Dungeon.h"
#include "game/actors/Char.h"
#include "game/actors/buffs/Buff.h"
#include "game/actors/buffs/Paralysis.h"

namespace pdv {

void ParalyticGas::evolve() {
    Blob::evolve();

    if (!dungeon()) {
        return;
    }

    Dungeon& game = *dungeon();

    for (int i = 0;
         i < LevelLength;
         ++i) {

        if (cur_[
                static_cast<std::size_t>(
                    i
                )
            ] <= 0) {
            continue;
        }

        Char* ch =
            game.scheduler().findChar(i);

        if (!ch) {
            continue;
        }

        Buff::prolong<Paralysis>(
            *ch,
            Paralysis::duration(*ch)
        );
    }
}

} // namespace pdv
