#include "game/actors/mobs/Mimic.h"
#include "game/Dungeon.h"
#include "game/actors/Hero.h"
#include "game/items/Gold.h"
#include "game/items/Item.h"
#include "game/levels/Level.h"
#include "game/utils/Random.h"
namespace pdv {
Mimic::Mimic(){name="mimic";state_=State::Hunting;enemySeen_=true;}
void Mimic::adjustStats(int level){level_=level;HT=(3+level)*4;HP=HT;exp_=2+2*(level-1)/5;defenseSkill_=(9+level)/2;enemySeen_=true;}
void Mimic::addItem(std::unique_ptr<Item>item){if(item)items_.push_back(std::move(item));}
int Mimic::damageRoll()const{return Random::NormalIntRange(HT/10,HT/4);}
int Mimic::attackSkill(const Char&)const{return 9+level_;}
int Mimic::attackProc(Char&enemy,int damage){if(dungeon_ && &enemy==&dungeon_->hero()&&Random::Int(3)==0){const int stolen=Random::Int(dungeon_->gold()/10,dungeon_->gold()/2);if(stolen>0&&dungeon_->spendGold(stolen)){std::unique_ptr<Gold>gold(new Gold(stolen));dungeon_->level().drop(std::move(gold),enemy.pos);}}return Mob::attackProc(enemy,damage);}
void Mimic::die(const void*cause){if(dungeon_){for(auto&item:items_)if(item)dungeon_->level().drop(std::move(item),pos);items_.clear();}Mob::die(cause);}
const char*Mimic::description()const{return"Mimics are magical creatures which take the shape of treasure chests to lure adventurers.";}
}
