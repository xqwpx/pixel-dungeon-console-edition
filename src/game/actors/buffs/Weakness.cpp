#include "game/actors/buffs/Weakness.h"
#include "game/actors/Hero.h"

namespace pdv {

bool Weakness::attachTo(Char& target) {
    Hero* hero = dynamic_cast<Hero*>(&target);

    if (!hero) {
        return false;
    }

    if (!FlavourBuff::attachTo(target)) {
        return false;
    }

    hero->setWeakened(true);

    // Original also discharges belongings; full discharge semantics will be
    // connected when the remaining chargeable equipment families are ported.
    return true;
}

void Weakness::detach() {
    Hero* hero =
        dynamic_cast<Hero*>(target());

    if (hero) {
        hero->setWeakened(false);
    }

    FlavourBuff::detach();
}

} // namespace pdv
