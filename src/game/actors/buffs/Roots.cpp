#include "game/actors/buffs/Roots.h"

namespace pdv {

bool Roots::attachTo(Char& target) {
    if (target.flying) {
        return false;
    }

    if (!FlavourBuff::attachTo(target)) {
        return false;
    }

    target.rooted = true;
    return true;
}

void Roots::detach() {
    Char* ch = target();

    if (ch) {
        ch->rooted = false;
    }

    FlavourBuff::detach();
}

} // namespace pdv
