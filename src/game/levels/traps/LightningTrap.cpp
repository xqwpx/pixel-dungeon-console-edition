#include "game/levels/traps/LightningTrap.h"
#include "game/actors/Char.h"
#include "game/utils/Random.h"
#include <algorithm>
namespace pdv { void LightningTrap::trigger(Dungeon&,int,Char* ch){ if(!ch)return; const int hp=std::max(1,ch->HP); int damage=std::max(1,Random::Int(hp/3,2*hp/3)); if(ch->resistantTo(typeid(LightningTrap)))damage=std::max(1,damage/2); ch->damage(damage,nullptr); } }
