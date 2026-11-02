#include "game/actors/mobs/SpecialMob.h"
#include "game/Dungeon.h"
#include "game/Progression.h"
#include "game/actors/Hero.h"
#include "game/actors/buffs/Amok.h"
#include "game/actors/buffs/Burning.h"
#include "game/actors/buffs/Paralysis.h"
#include "game/actors/buffs/Poison.h"
#include "game/actors/buffs/Roots.h"
#include "game/actors/buffs/Terror.h"
#include "game/actors/mobs/npcs/DungeonNPC.h"
#include "game/actors/blobs/Blob.h"
#include "game/actors/blobs/ToxicGas.h"
#include "game/items/Generator.h"
#include "game/items/SpecialItems.h"
#include "game/items/weapon/Weapon.h"
#include "game/levels/Level.h"
#include "game/utils/Random.h"
#include <algorithm>
#include <vector>
namespace pdv {
SpecialMob::~SpecialMob()=default;
SpecialMob::SpecialMob(SpecialMobKind kind,int depth,bool createEquipment):kind_(kind),depth_(std::max(1,depth)){
 switch(kind_){
 case SpecialMobKind::Wraith:name="wraith";HP=HT=1;defenseSkill_=(10+depth_)*5;exp_=0;flying=true;state_=State::Hunting;enemySeen_=true;break;
 case SpecialMobKind::Statue:name="animated statue";HP=HT=15+depth_*5;defenseSkill_=4+depth_;exp_=0;state_=State::Passive;if(createEquipment){for(int tries=0;tries<64&&!weapon_;++tries){std::unique_ptr<Item>item=Generator::random(Generator::Category::Weapon,depth_);Weapon*w=dynamic_cast<Weapon*>(item.get());if(w&&!w->isMissileWeapon()&&w->level()>=0){item.release();weapon_.reset(w);weapon_->identify();weapon_->enchant();}}}break;
 case SpecialMobKind::FetidRat:name="fetid rat";HP=HT=15;defenseSkill_=5;exp_=3;maxLvl_=5;state_=State::Wandering;break;
 case SpecialMobKind::CursePersonification:name="curse personification";HP=HT=10+depth_*3;defenseSkill_=10+depth_;exp_=3;maxLvl_=5;state_=State::Hunting;baseSpeed_=0.5f;flying=true;break;
 case SpecialMobKind::Bee:name="golden bee";HP=HT=(3+depth_)*5;defenseSkill_=9+depth_;exp_=0;viewDistance=4;flying=true;hostile_=false;state_=State::Wandering;break;
 }
}
int SpecialMob::damageRoll()const{switch(kind_){case SpecialMobKind::Wraith:return Random::NormalIntRange(1,3+depth_);case SpecialMobKind::Statue:return weapon_?Random::NormalIntRange(weapon_->min(),weapon_->max()):1;case SpecialMobKind::FetidRat:return Random::NormalIntRange(2,6);case SpecialMobKind::CursePersonification:return Random::NormalIntRange(3,5);case SpecialMobKind::Bee:return Random::NormalIntRange(HT/10,HT/4);}return 1;}
int SpecialMob::attackSkill(const Char&)const{switch(kind_){case SpecialMobKind::Wraith:return 10+depth_;case SpecialMobKind::Statue:return static_cast<int>((9+depth_)*(weapon_?weapon_->ACU():1.0f));case SpecialMobKind::FetidRat:return 12;case SpecialMobKind::CursePersonification:return 10+depth_;case SpecialMobKind::Bee:return defenseSkill_;}return 0;}
int SpecialMob::dr()const{return kind_==SpecialMobKind::Statue?depth_:(kind_==SpecialMobKind::FetidRat?2:(kind_==SpecialMobKind::CursePersonification?1:0));}
float SpecialMob::attackDelay()const{return kind_==SpecialMobKind::Statue&&weapon_?weapon_->DLY():1.0f;}
bool SpecialMob::act(){if(kind_==SpecialMobKind::Statue&&dungeon_&&dungeon_->visible()[static_cast<std::size_t>(pos)])Progression::instance().addJournal(JournalFeature::Statue,dungeon_->depth());if(kind_==SpecialMobKind::Bee){if(--HP<=0){die(nullptr);return true;}enemy_=chooseEnemy();if(!enemy_&&dungeon_){enemySeen_=false;if(getCloser(dungeon_->hero().pos)){spend(TICK/speed());next();}else{spend(TICK);next();}return true;}}if(kind_==SpecialMobKind::CursePersonification&&HP>0&&HP<HT)++HP;return Mob::act();}
void SpecialMob::damage(int amount,const void*source){if(kind_==SpecialMobKind::Statue&&state_==State::Passive)state_=State::Hunting;Mob::damage(amount,source);}
void SpecialMob::beckon(int cell){if(kind_!=SpecialMobKind::Statue)Mob::beckon(cell);}
int SpecialMob::defenseProc(Char&enemy,int damage){if(kind_==SpecialMobKind::FetidRat&&dungeon_)dungeon_->level().seedBlob(BlobKind::ParalyticGas,pos,20);return Mob::defenseProc(enemy,damage);}
int SpecialMob::attackProc(Char&enemy,int damage){if(kind_==SpecialMobKind::Statue&&weapon_)weapon_->proc(*this,enemy,damage);else if(kind_==SpecialMobKind::CursePersonification&&dungeon_){const int destination=enemy.pos+(enemy.pos-pos);if(dungeon_->level().inBoundsCell(destination)&&(dungeon_->level().passable(destination)||dungeon_->level().avoid(destination))&&!dungeon_->scheduler().findChar(destination))dungeon_->relocateChar(enemy,destination);}else if(kind_==SpecialMobKind::Bee){if(Mob*mob=dynamic_cast<Mob*>(&enemy))mob->beckon(pos);}return Mob::attackProc(enemy,damage);}
Char*SpecialMob::chooseEnemy(){if(kind_!=SpecialMobKind::Bee)return Mob::chooseEnemy();if(enemy_&&enemy_->isAlive()&&(!dungeon_||enemy_!=&dungeon_->hero()))return enemy_;std::vector<Mob*>targets;if(dungeon_)for(auto&mob:dungeon_->level().mobsMutable())if(mob&&mob.get()!=this&&mob->isAlive()&&mob->hostile()&&dungeon_->level().fieldOfView()[static_cast<std::size_t>(mob->pos)])targets.push_back(mob.get());return Random::element(targets);}
bool SpecialMob::immuneTo(const std::type_info&type)const{if(kind_==SpecialMobKind::Wraith&&type==typeid(Terror))return true;if(kind_==SpecialMobKind::FetidRat&&type==typeid(Paralysis))return true;if(kind_==SpecialMobKind::CursePersonification&&(type==typeid(Terror)||type==typeid(Paralysis)||type==typeid(Roots)))return true;if(kind_==SpecialMobKind::Bee&&type==typeid(Poison))return true;return Mob::immuneTo(type);}
bool SpecialMob::resistantTo(const std::type_info& type) const {
    return kind_ == SpecialMobKind::Statue && (type == typeid(ToxicGas) || type == typeid(Poison));
}
void SpecialMob::die(const void*cause){if(dungeon_&&kind_==SpecialMobKind::Statue){Progression::instance().removeJournal(JournalFeature::Statue,dungeon_->depth());if(weapon_)dungeon_->level().drop(std::unique_ptr<Item>(weapon_.release()),pos);}if(dungeon_&&kind_==SpecialMobKind::FetidRat)dungeon_->level().drop(std::unique_ptr<Item>(new SpecialItem(SpecialKind::RatSkull)),pos);if(dungeon_&&kind_==SpecialMobKind::CursePersonification){dungeon_->setQuestState(0,350);std::unique_ptr<DungeonNPC>ghost(new DungeonNPC(NPCKind::Ghost));DungeonNPC*raw=ghost.get();ghost->pos=pos;ghost->setState(Mob::State::Passive);ghost->bind(dungeon_);dungeon_->level().addMob(std::move(ghost));dungeon_->scheduler().add(raw);}Mob::die(cause);}
const char*SpecialMob::description()const{switch(kind_){case SpecialMobKind::Wraith:return"A vengeful ethereal spirit, extremely hard to hit with ordinary weapons.";case SpecialMobKind::Statue:return"An animated stone statue wielding a real enchanted weapon.";case SpecialMobKind::FetidRat:return"A huge rat surrounded by a foul paralytic cloud.";case SpecialMobKind::CursePersonification:return"A living personification of the sad ghost's curse.";case SpecialMobKind::Bee:return"A short-lived golden bee which fiercely protects its master.";}return"";}
}
