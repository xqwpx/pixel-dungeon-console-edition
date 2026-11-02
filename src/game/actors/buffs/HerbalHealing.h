#pragma once
#include "game/actors/buffs/Buff.h"
namespace pdv { class HerbalHealing:public Buff{public:bool attachTo(Char&target)override;bool act()override;private:int pos_=-1;}; }
