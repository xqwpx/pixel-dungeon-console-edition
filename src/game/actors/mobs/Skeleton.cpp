#include "game/actors/mobs/Skeleton.h"
#include "game/Dungeon.h"
#include "game/utils/Random.h"
#include "game/items/Generator.h"
#include <algorithm>

namespace pdv {
Skeleton::Skeleton(){name="skeleton";HP=HT=18;defenseSkill_=7;exp_=5;maxLvl_=10;}
int Skeleton::damageRoll()const{return Random::NormalIntRange(3,8);}
int Skeleton::attackSkill(const Char&)const{return 12;}
int Skeleton::dr()const{return 3;}
std::unique_ptr<Item>Skeleton::createLoot()const{std::unique_ptr<Item>loot=Generator::random(Generator::Category::Weapon,dungeon_?dungeon_->depth():1);for(int i=0;i<2;++i){std::unique_ptr<Item>candidate=Generator::random(Generator::Category::Weapon,dungeon_?dungeon_->depth():1);if(candidate&&(!loot||candidate->level()<loot->level()))loot=std::move(candidate);}return loot;}
void Skeleton::die(const void* cause){
 Mob::die(cause);
 if(!dungeon_)return;
 for(int offset:Level::Neighbours8){
  Char* target=dungeon_->scheduler().findChar(pos+offset);
  if(target&&target->isAlive())target->damage(std::max(0,damageRoll()-Random::IntRange(0,target->dr()/2)),this);
 }
}
const char* Skeleton::description()const{return "Animated bones that explode into dangerous fragments when destroyed.";}
}
