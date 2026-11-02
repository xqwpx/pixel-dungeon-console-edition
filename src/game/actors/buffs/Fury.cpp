#include "game/actors/buffs/Fury.h"
#include "game/actors/Char.h"

namespace pdv {
bool Fury::act() {
    if (!target_ || target_->HP > target_->HT * Level) { detach(); return true; }
    spend(TICK); next(); return true;
}
}
