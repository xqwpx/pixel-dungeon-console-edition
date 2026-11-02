#include "game/actors/buffs/HerbalHealing.h"
#include "game/actors/Char.h"
#include <algorithm>
namespace pdv { bool HerbalHealing::attachTo(Char&target){pos_=target.pos;return Buff::attachTo(target);}bool HerbalHealing::act(){if(!target_||target_->pos!=pos_||target_->HP>=target_->HT){detach();return true;}target_->HP=std::min(target_->HT,target_->HP+std::max(1,target_->HT/10));spend(5.0f);next();return true;} }
