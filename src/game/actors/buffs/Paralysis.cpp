#include "game/actors/buffs/Paralysis.h"
#include "game/actors/buffs/Frost.h"

namespace pdv {

bool Paralysis::attachTo(Char& target) {
    if (!FlavourBuff::attachTo(target)) {
        return false;
    }

    target.paralysed = true;
    return true;
}

void Paralysis::detach() {
    Char* ch = target();
    FlavourBuff::detach();

    if (ch) {
        unfreeze(*ch);
    }
}

void Paralysis::unfreeze(Char& target) {
    target.paralysed =
        target.hasBuff(typeid(Paralysis)) ||
        target.hasBuff(typeid(Frost));
}

} // namespace pdv
