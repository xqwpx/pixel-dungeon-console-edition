#include "game/actors/mobs/Thief.h"
#include "game/Dungeon.h"
#include "game/actors/Hero.h"
#include "game/actors/buffs/Terror.h"
#include "game/items/Item.h"
#include "game/items/EquipmentCatalog.h"
#include "game/items/Gold.h"
#include "game/items/Bag.h"
#include "game/utils/Random.h"
#include <vector>

namespace pdv {
Thief::Thief(bool bandit):bandit_(bandit){name=bandit_?"crazy bandit":"crazy thief";HP=HT=20;defenseSkill_=12;exp_=5;maxLvl_=10;}
int Thief::damageRoll()const{return Random::NormalIntRange(1,7);}
int Thief::attackSkill(const Char&)const{return 12;}
int Thief::dr()const{return 3;}
int Thief::attackProc(Char& enemy,int damage){
 Hero* hero=dynamic_cast<Hero*>(&enemy);
 if(hero&&!stolen_&&!hero->belongings().backpack().items().empty()){
  auto& items=hero->belongings().backpack().itemsMutable();
  std::vector<Item*> candidates;
  for(const auto& item:items)if(item&&!item->unique()&&!dynamic_cast<Bag*>(item.get()))candidates.push_back(item.get());
  Item* chosen=Random::element(candidates);
  if(!chosen)return damage;
  stolen_=hero->belongings().backpack().removeAll(chosen);
  if(stolen_){if(dungeon_)dungeon_->postMessage(name+" stole "+stolen_->displayName()+" from you!");setState(State::Fleeing);}
 }
 return damage;
}
int Thief::defenseProc(Char&enemy,int damage){if(stolen_&&state_==State::Fleeing&&dungeon_)dungeon_->level().drop(std::unique_ptr<Item>(new Gold()),pos);return Mob::defenseProc(enemy,damage);}
bool Thief::actFleeing(bool enemyInFOV){enemySeen_=enemyInFOV;if(enemyInFOV&&enemy_)target_=enemy_->pos;if(target_!=-1&&getFurther(target_)){spend(TICK/speed());next();return true;}if(!hasBuff(typeid(Terror))){state_=State::Hunting;target_=enemy_?enemy_->pos:target_;}spend(TICK);next();return true;}
void Thief::die(const void* cause){Mob::die(cause);if(dungeon_&&stolen_)dungeon_->level().drop(std::move(stolen_),pos);}
std::unique_ptr<Item>Thief::createLoot()const{return std::unique_ptr<Item>(new CatalogRing(RingKind::Haggler));}
const char* Thief::description()const{return stolen_?"A crazed thief fleeing with an item stolen from you.":"A fast criminal hiding in the dungeon.";}
}
