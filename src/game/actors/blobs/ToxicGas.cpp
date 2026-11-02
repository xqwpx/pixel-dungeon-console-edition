#include "game/actors/blobs/ToxicGas.h"

#include "game/Dungeon.h"
#include "game/actors/Char.h"
#include "game/actors/Hero.h"
#include "game/actors/buffs/GasesImmunity.h"
#include "game/actors/blobs/ParalyticGas.h"
#include "game/utils/Random.h"
#include "game/Progression.h"
#include <algorithm>

namespace pdv {

void ToxicGas::evolve() {
    Blob::evolve();

    if (!dungeon()) {
        return;
    }

    Dungeon& game = *dungeon();

    const int levelDamage =
        5 +
        game.depth() * 5;

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

        if (!ch ||
            ch->hasBuff(
                typeid(GasesImmunity)
            )||ch->immuneTo(typeid(ToxicGas))) {
            continue;
        }

        const int raw =
            ch->HT +
            levelDamage;

        int damage =
            raw / 40;

        if (Random::Int(40) <
            raw % 40) {
            ++damage;
        }

        if(ch->resistantTo(typeid(ToxicGas))) damage=std::max(1,damage/2);
        if(dynamic_cast<Hero*>(ch)&&damage>=ch->HP)Progression::instance().noteDeath(Badge::DeathGas);
        ch->damage(damage,this);
    }

    Blob* other =
        game.level().blob(
            BlobKind::ParalyticGas
        );

    ParalyticGas* par =
        dynamic_cast<ParalyticGas*>(
            other
        );

    if (!par) {
        return;
    }

    for (int i = 0;
         i < LevelLength;
         ++i) {

        const int t =
            cur_[
                static_cast<std::size_t>(
                    i
                )
            ];

        const int p =
            par->valueAt(i);

        if (t <= 0 ||
            p <= 0) {
            continue;
        }

        if (p >= t) {
            setValueAt(i, 0);
        } else {
            par->setValueAt(i, 0);
        }
    }
}

} // namespace pdv
