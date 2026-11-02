#include "game/actors/buffs/Invisibility.h"

namespace pdv {

bool Invisibility::attachTo(Char& target) {
    if (!FlavourBuff::attachTo(target)) {
        return false;
    }

    target.invisible++;
    return true;
}

void Invisibility::detach() {
    Char* ch = target();

    if (ch && ch->invisible > 0) {
        ch->invisible--;
    }

    FlavourBuff::detach();
}

} // namespace pdv
