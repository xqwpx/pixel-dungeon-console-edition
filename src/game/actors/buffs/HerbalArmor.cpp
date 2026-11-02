#include "game/actors/buffs/HerbalArmor.h"
#include "game/actors/Char.h"
#include <algorithm>
namespace pdv { bool HerbalArmor::attachTo(Char&target){pos_=target.pos;return Buff::attachTo(target);}bool HerbalArmor::act(){if(!target_||target_->pos!=pos_){detach();return true;}spend(TICK);next();return true;}int HerbalArmor::absorb(int damage){const int left=std::max(0,damage-level_);level_=std::max(0,level_-damage);if(level_==0)detach();return left;} }
