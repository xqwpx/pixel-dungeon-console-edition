#pragma once
#include "game/actors/buffs/Buff.h"
namespace pdv { class HerbalArmor:public Buff{public:bool attachTo(Char&target)override;bool act()override;void level(int value){if(value>level_)level_=value;}int absorb(int damage);int persistentInt()const override{return level_;}void restorePersistent(float c,float v,int s)override{level_=s;Buff::restorePersistent(c,v,s);}private:int pos_=-1;int level_=0;}; }
