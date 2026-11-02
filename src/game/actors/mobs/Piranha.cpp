#include "game/actors/mobs/Piranha.h"
#include "game/Dungeon.h"
#include "game/utils/Random.h"
#include "game/actors/buffs/Burning.h"
#include "game/actors/buffs/Paralysis.h"
#include "game/actors/buffs/Roots.h"
#include "game/actors/buffs/Frost.h"
#include "game/actors/blobs/ToxicGas.h"
#include "game/Progression.h"
#include "game/items/ConsumableCatalog.h"

namespace pdv {
Piranha::Piranha(int depth):depth_(depth){name="giant piranha";baseSpeed_=1.0f;exp_=0;HP=HT=10+depth_*5;defenseSkill_=10+depth_*2;}
bool Piranha::act(){if(!dungeon_||!dungeon_->level().water(pos)){die(nullptr);return true;}return Mob::act();}
void Piranha::die(const void*cause){if(dungeon_)dungeon_->level().drop(std::unique_ptr<Item>(new MysteryMeat()),pos);Progression::instance().notePiranha();Mob::die(cause);}
int Piranha::damageRoll()const{return Random::NormalIntRange(depth_,4+depth_*2);}
int Piranha::attackSkill(const Char&)const{return 20+depth_*2;}
int Piranha::dr()const{return depth_;}
bool Piranha::immuneTo(const std::type_info&type)const{return type==typeid(Burning)||type==typeid(Paralysis)||type==typeid(Roots)||type==typeid(Frost)||type==typeid(ToxicGas)||Mob::immuneTo(type);}
bool Piranha::getCloser(int target){
    if(rooted||!dungeon_) return false;
    int best=-1;
    int bestDistance=Level::distance(pos,target);
    for(int offset:Level::Neighbours8){const int cell=pos+offset;if(!dungeon_->level().inBoundsCell(cell)||!dungeon_->level().water(cell)||dungeon_->scheduler().findChar(cell))continue;const int distance=Level::distance(cell,target);if(distance<bestDistance){bestDistance=distance;best=cell;}}
    return best>=0&&moveTo(best,dungeon_->level());
}
bool Piranha::getFurther(int target){
    if(!dungeon_)return false;
    int best=-1;
    int bestDistance=Level::distance(pos,target);
    for(int offset:Level::Neighbours8){const int cell=pos+offset;if(!dungeon_->level().inBoundsCell(cell)||!dungeon_->level().water(cell)||dungeon_->scheduler().findChar(cell))continue;const int distance=Level::distance(cell,target);if(distance>bestDistance){bestDistance=distance;best=cell;}}
    return best>=0&&moveTo(best,dungeon_->level());
}
const char* Piranha::description()const{return "A carnivorous fish bred to protect flooded treasure rooms.";}
}
