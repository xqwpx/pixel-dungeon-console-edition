#include "game/actors/buffs/Levitation.h"

#include "game/actors/buffs/Buff.h"
#include "game/actors/buffs/Roots.h"
#include "game/levels/Level.h"

namespace pdv {

bool Levitation::attachTo(Char& target) {
    if (!FlavourBuff::attachTo(target)) {
        return false;
    }

    target.flying = true;
    Buff::detach<Roots>(target);

    return true;
}

void Levitation::detach() {
    Char* ch = target();

    if (ch) {
        ch->flying = false;
    }

    FlavourBuff::detach();

    // Original Levitation.detach(): landing presses the cell underneath.
    if (ch) {
        if (Level* level = ch->levelContext()) {
            level->press(ch->pos, ch);
        }
    }
}

} // namespace pdv
