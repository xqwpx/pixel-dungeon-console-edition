#include "game/actors/buffs/Terror.h"

namespace pdv {

void Terror::recover(Char& target) {
    Terror* terror =
        dynamic_cast<Terror*>(
            target.findBuff(
                typeid(Terror)
            )
        );

    if (terror &&
        terror->cooldown() < Duration) {
        terror->detach();
    }
}

} // namespace pdv
