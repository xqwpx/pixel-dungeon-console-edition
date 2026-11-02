#include "game/actors/buffs/Barkskin.h"
#include "game/actors/Char.h"
namespace pdv { bool Barkskin::act(){if(!target_||!target_->isAlive()||--level_<=0){detach();return true;}spend(TICK);next();return true;} }
