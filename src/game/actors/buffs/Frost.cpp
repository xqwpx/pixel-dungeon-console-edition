#include "game/actors/buffs/Frost.h"

#include "game/actors/buffs/Buff.h"
#include "game/actors/buffs/Burning.h"
#include "game/actors/buffs/Paralysis.h"

namespace pdv {

bool Frost::attachTo(Char& target) {
    if (!FlavourBuff::attachTo(target)) {
        return false;
    }

    target.paralysed = true;

    // Original Frost immediately extinguishes Burning.
    Buff::detach<Burning>(target);

    // MysteryMeat -> FrozenCarpaccio depends on that food family.
    return true;
}

void Frost::detach() {
    Char* ch = target();

    FlavourBuff::detach();

    if (ch) {
        Paralysis::unfreeze(*ch);
    }
}

} // namespace pdv
