#pragma once
#include "game/actors/buffs/Buff.h"
namespace pdv { class Barkskin:public Buff{public:int level()const{return level_;}void level(int value){if(value>level_)level_=value;}bool act()override;int persistentInt()const override{return level_;}void restorePersistent(float c,float v,int s)override{level_=s;Buff::restorePersistent(c,v,s);}private:int level_=0;}; }
