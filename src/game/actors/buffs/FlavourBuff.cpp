#include "game/actors/buffs/FlavourBuff.h"

namespace pdv {

bool FlavourBuff::act() {
    detach();
    return true;
}

} // namespace pdv
