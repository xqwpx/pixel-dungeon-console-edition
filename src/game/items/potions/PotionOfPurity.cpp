#include "game/items/potions/PotionOfPurity.h"
#include "game/items/ItemSpriteSheet.h"

#include "game/Dungeon.h"
#include "game/actors/Hero.h"
#include "game/actors/blobs/Blob.h"
#include "game/actors/buffs/Buff.h"
#include "game/actors/buffs/GasesImmunity.h"
#include "game/pathfinding/PathFinder.h"

namespace pdv {

PotionOfPurity::PotionOfPurity() {
    image_ = ItemSpriteSheet::POTION_AMBER;
    name_ = "Potion of Purification";
}

void PotionOfPurity::apply(
    Dungeon&,
    Hero& hero
) {
    setKnown();

    Buff::prolong<GasesImmunity>(
        hero,
        GasesImmunity::Duration
    );
}

void PotionOfPurity::shatter(
    Dungeon& dungeon,
    int cell
) {
    std::array<bool, Level::Length> pass{};

    for (int i = 0;
         i < Level::Length;
         ++i) {

        pass[
            static_cast<std::size_t>(
                i
            )
        ] =
            !dungeon
                .level()
                .losBlocking(i);
    }

    PathFinder::buildDistanceMap(
        cell,
        pass,
        2
    );

    const auto& distance =
        PathFinder::distance();

    Blob* toxic =
        dungeon.level().blob(
            BlobKind::ToxicGas
        );

    Blob* paralytic =
        dungeon.level().blob(
            BlobKind::ParalyticGas
        );

    bool changed = false;

    for (int i = 0;
         i < Level::Length;
         ++i) {

        if (i >=
                static_cast<int>(
                    distance.size()
                ) ||
            distance[
                static_cast<std::size_t>(
                    i
                )
            ] > 2) {
            continue;
        }

        if (toxic &&
            toxic->valueAt(i) > 0) {
            toxic->clear(i);
            changed = true;
        }

        if (paralytic &&
            paralytic->valueAt(i) > 0) {
            paralytic->clear(i);
            changed = true;
        }
    }

    if (changed) {
        setKnown();
    }
}

std::unique_ptr<Item>
PotionOfPurity::clone() const {
    return std::unique_ptr<Item>(
        new PotionOfPurity(*this)
    );
}

} // namespace pdv
