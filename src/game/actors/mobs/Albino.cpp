#include "game/actors/mobs/Albino.h"

#include "game/actors/buffs/Bleeding.h"
#include "game/actors/buffs/Buff.h"
#include "game/utils/Random.h"

namespace pdv {

Albino::Albino() {
    name = "albino rat";
    HP = HT = 15;
}

int Albino::attackProc(Char& enemy, int damage) {
    if (Random::Int(2) == 0) {
        if (Bleeding* bleeding = Buff::affect<Bleeding>(enemy)) bleeding->set(damage);
    }
    return damage;
}

} // namespace pdv
