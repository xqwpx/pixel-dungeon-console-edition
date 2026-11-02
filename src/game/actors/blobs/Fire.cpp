#include "game/actors/blobs/Fire.h"

#include "game/Dungeon.h"
#include "game/actors/Char.h"
#include "game/actors/buffs/Buff.h"
#include "game/actors/buffs/Burning.h"
#include "game/levels/Level.h"
#include "game/items/Heap.h"

namespace pdv {

void Fire::evolve() {
    if (!dungeon()) {
        return;
    }

    Level& level =
        dungeon()->level();

    for (int pos =
             Level::Width + 1;
         pos <
             Level::Length -
             Level::Width - 1;
         ++pos) {

        int fire = 0;

        if (cur_[
                static_cast<std::size_t>(
                    pos
                )
            ] > 0) {

            burn(pos);

            fire =
                cur_[
                    static_cast<std::size_t>(
                        pos
                    )
                ] - 1;

            if (fire <= 0 &&
                level.flamable(pos)) {

                level.destroy(pos);
            }

        } else if (
            level.flamable(pos) &&
            (
                cur_[static_cast<std::size_t>(pos - 1)] > 0 ||
                cur_[static_cast<std::size_t>(pos + 1)] > 0 ||
                cur_[static_cast<std::size_t>(pos - Level::Width)] > 0 ||
                cur_[static_cast<std::size_t>(pos + Level::Width)] > 0
            )
        ) {

            fire = 4;
            burn(pos);
        }

        off_[
            static_cast<std::size_t>(
                pos
            )
        ] = fire;

        volume_ += fire;
    }

    level.buildFlagMaps();
    level.cleanWalls();
}

void Fire::burn(int pos) {
    if (!dungeon()) {
        return;
    }

    Char* ch =
        dungeon()
            ->scheduler()
            .findChar(pos);

    if (ch) {
        Burning* burning =
            Buff::affect<Burning>(*ch);

        if (burning) {
            burning->reignite(*ch);
        }
    }

    if(Heap* heap=dungeon()->level().heapAt(pos)){
        if(heap->type==Heap::Type::Mimic)dungeon()->activateMimicAt(pos,true);
        else {heap->burn();dungeon()->level().removeEmptyHeap(pos);}
    }
}

void Fire::seed(
    int cell,
    int amount
) {
    if (cell < 0 ||
        cell >= LevelLength ||
        amount <= 0) {
        return;
    }

    if (cur_[
            static_cast<std::size_t>(
                cell
            )
        ] == 0) {

        cur_[
            static_cast<std::size_t>(
                cell
            )
        ] = amount;

        volume_ += amount;
    }
}

} // namespace pdv
