#include "game/actors/buffs/Blindness.h"

namespace pdv {

void Blindness::detach() {
    // Original Blindness.detach() asks Dungeon.observe() to refresh vision.
    // The Vita Dungeon refreshes FOV after every completed player turn, so
    // detaching here only removes the logical state.
    FlavourBuff::detach();
}

} // namespace pdv
