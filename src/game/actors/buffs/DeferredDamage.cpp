#include "game/actors/buffs/DeferredDamage.h"
#include "game/actors/Hero.h"
#include "game/Progression.h"
#include <algorithm>
namespace pdv {
bool DeferredDamage::attachTo(Char& target){if(!Buff::attachTo(target))return false;postpone(TICK);return true;}
bool DeferredDamage::act(){Char*ch=target();if(!ch||!ch->isAlive()||damage_<=0){detach();return true;}ch->damage(1,this);if(!ch->isAlive()&&dynamic_cast<Hero*>(ch))Progression::instance().noteDeath(Badge::DeathGlyph);--damage_;if(damage_<=0||!ch->isAlive())detach();else{spend(TICK);next();}return true;}
void DeferredDamage::restorePersistent(float cooldownValue,float,int state){damage_=std::max(0,state);Buff::restorePersistent(cooldownValue,0.0f,state);}
}
