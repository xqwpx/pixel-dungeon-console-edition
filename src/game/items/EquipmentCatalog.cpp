#include "game/items/EquipmentCatalog.h"
#include "game/Dungeon.h"
#include "game/actors/buffs/Amok.h"
#include "game/actors/buffs/Burning.h"
#include "game/actors/buffs/Poison.h"
#include "game/actors/buffs/Slow.h"
#include "game/actors/buffs/Vertigo.h"
#include "game/actors/buffs/Paralysis.h"
#include "game/actors/buffs/Cripple.h"
#include "game/actors/buffs/Bleeding.h"
#include "game/items/ItemSpriteSheet.h"
#include "game/items/Heap.h"
#include "game/actors/mobs/Mob.h"
#include "game/actors/mobs/ZoneMob.h"
#include "game/pathfinding/PathFinder.h"
#include "game/utils/Random.h"
#include "game/actors/blobs/Blob.h"
#include "game/levels/Terrain.h"
#include "game/mechanics/Ballistica.h"
#include <unordered_set>

namespace pdv {
namespace {
struct MeleeDef{int tier;float acu,delay;const char*name;int image;};
const MeleeDef meleeDefs[]={{2,1,1,"quarterstaff",ItemSpriteSheet::QUARTERSTAFF},{2,1,1.5f,"spear",ItemSpriteSheet::SPEAR},{3,1,0.8f,"mace",ItemSpriteSheet::MACE},{3,1,1,"sword",ItemSpriteSheet::SWORD},{4,1,1,"longsword",ItemSpriteSheet::LONG_SWORD},{4,1.2f,1,"battle axe",ItemSpriteSheet::BATTLE_AXE},{5,1.2f,1,"war hammer",ItemSpriteSheet::WAR_HAMMER},{5,1,1,"glaive",ItemSpriteSheet::GLAIVE}};
}
CatalogMelee::CatalogMelee(MeleeKind kind):MeleeWeapon(meleeDefs[static_cast<int>(kind)].tier,meleeDefs[static_cast<int>(kind)].acu,meleeDefs[static_cast<int>(kind)].delay),kind_(kind){const auto&d=meleeDefs[static_cast<int>(kind)];name_=d.name;image_=d.image;}
std::unique_ptr<Item> CatalogMelee::clone()const{return std::unique_ptr<Item>(new CatalogMelee(*this));}
CatalogMissile::CatalogMissile(MissileKind kind):kind_(kind){stackable_=true;switch(kind){case MissileKind::Javelin:name_="javelin";image_=ItemSpriteSheet::JAVELIN;strength_=15;min_=2;max_=15;break;case MissileKind::IncendiaryDart:name_="incendiary dart";image_=ItemSpriteSheet::INCENDIARY_DART;strength_=12;min_=1;max_=2;break;case MissileKind::CurareDart:name_="curare dart";image_=ItemSpriteSheet::CURARE_DART;strength_=14;min_=1;max_=3;break;case MissileKind::Shuriken:name_="shuriken";image_=ItemSpriteSheet::SHURIKEN;strength_=13;min_=2;max_=6;break;case MissileKind::Tomahawk:name_="tomahawk";image_=ItemSpriteSheet::TOMAHAWK;strength_=17;min_=4;max_=20;break;}}
std::unique_ptr<Item> CatalogMissile::clone()const{return std::unique_ptr<Item>(new CatalogMissile(*this));}
float CatalogMissile::speedFactor(const Hero&hero)const{return Weapon::speedFactor(hero)*(kind_==MissileKind::Shuriken?0.5f:1.0f);}
void CatalogMissile::proc(Char&attacker,Char&defender,int damage){Weapon::proc(attacker,defender,damage);switch(kind_){case MissileKind::Javelin:Buff::prolong<Cripple>(defender,Cripple::Duration);break;case MissileKind::IncendiaryDart:if(Burning*b=Buff::affect<Burning>(defender))b->reignite(defender);break;case MissileKind::CurareDart:Buff::prolong<Paralysis>(defender,3.0f*elementsDurationFactor(defender));break;case MissileKind::Tomahawk:if(Bleeding*b=Buff::affect<Bleeding>(defender))b->set(damage);break;default:break;}}
bool CatalogMissile::onMiss(Dungeon&d,int cell){if(kind_==MissileKind::IncendiaryDart&&d.level().flamable(cell)){d.level().seedBlob(BlobKind::Fire,cell,4);return true;}return false;}
int CatalogMissile::price()const{switch(kind_){case MissileKind::IncendiaryDart:return 10*quantity();case MissileKind::CurareDart:return 12*quantity();case MissileKind::Javelin:case MissileKind::Shuriken:return 15*quantity();case MissileKind::Tomahawk:return 20*quantity();}return 1;}
CatalogArmor::CatalogArmor(ArmorKind kind):Armor(static_cast<int>(kind)),kind_(kind){static const char*n[]={"","","leather armor","mail armor","scale armor","plate armor"};static const int i[]={0,0,25,26,27,28};name_=n[static_cast<int>(kind)];image_=i[static_cast<int>(kind)];}
std::unique_ptr<Item> CatalogArmor::clone()const{return std::unique_ptr<Item>(new CatalogArmor(*this));}
CatalogRing::CatalogRing(RingKind kind):kind_(kind){static const char*n[]={"Ring of Mending","Ring of Detection","Ring of Power","Ring of Herbalism","Ring of Accuracy","Ring of Evasion","Ring of Satiety","Ring of Haste","Ring of Elements","Ring of Haggler","Ring of Thorns"};static const int images[]={32,33,34,35,36,37,38,39,72,73,74};name_=n[static_cast<int>(kind)];image_=images[static_cast<int>(kind)];}
int CatalogRing::accuracyBonus()const{return kind_==RingKind::Accuracy?effectiveLevel():0;}
int CatalogRing::evasionBonus()const{return kind_==RingKind::Evasion?effectiveLevel():0;}
int CatalogRing::hasteBonus()const{return kind_==RingKind::Haste?effectiveLevel():0;}
int CatalogRing::satietyBonus()const{return kind_==RingKind::Satiety?effectiveLevel():0;}
int CatalogRing::mendingBonus()const{return kind_==RingKind::Mending?effectiveLevel():0;}
int CatalogRing::powerBonus()const{return kind_==RingKind::Power?effectiveLevel():0;}
int CatalogRing::detectionBonus()const{return kind_==RingKind::Detection?effectiveLevel():0;}
bool CatalogRing::detectsSecrets()const{return kind_==RingKind::Detection;}
int CatalogRing::herbalismBonus()const{return kind_==RingKind::Herbalism?effectiveLevel():0;}
int CatalogRing::elementsBonus()const{return kind_==RingKind::Elements?effectiveLevel():0;}
bool CatalogRing::haggling()const{return kind_==RingKind::Haggler;}
bool CatalogRing::thorns()const{return kind_==RingKind::Thorns;}
std::unique_ptr<Item> CatalogRing::clone()const{return std::unique_ptr<Item>(new CatalogRing(*this));}
CatalogWand::CatalogWand(WandKind kind):kind_(kind){static const char*n[]={"Wand of Teleportation","Wand of Slowness","Wand of Firebolt","Wand of Regrowth","Wand of Poison","Wand of Blink","Wand of Lightning","Wand of Amok","Wand of Reach","Wand of Flock","Wand of Disintegration","Wand of Avalanche"};static const int images[]={48,49,50,51,52,53,54,55,68,69,70,71};name_=n[static_cast<int>(kind)];image_=images[static_cast<int>(kind)];hitChars_=kind!=WandKind::Reach&&kind!=WandKind::Disintegration&&kind!=WandKind::Avalanche;maxCharges_=curCharges_=initialCharges();}
void CatalogWand::onZap(Dungeon&d,Hero&hero,int cell){
    Char*ch=d.scheduler().findChar(cell);Level&level=d.level();const auto&trace=Ballistica::trace();
    switch(kind_){
    case WandKind::Regrowth:{for(int i=1;i<Ballistica::distance()-1;++i){const int p=trace[static_cast<std::size_t>(i)];const int t=level.cell(p);if(t==Terrain::EMPTY||t==Terrain::EMBERS||t==Terrain::EMPTY_DECO)level.setRaw(p,Terrain::GRASS);}const int t=level.cell(cell);if(t==Terrain::EMPTY||t==Terrain::EMBERS||t==Terrain::EMPTY_DECO||t==Terrain::GRASS||t==Terrain::HIGH_GRASS)level.seedBlob(BlobKind::Regrowth,cell,(power()+2)*20);level.buildFlagMaps();d.observe();break;}
    case WandKind::Firebolt:{for(int i=1;i<Ballistica::distance();++i){const int p=trace[static_cast<std::size_t>(i)];if(level.flamable(p))level.seedBlob(BlobKind::Fire,p,1);}level.seedBlob(BlobKind::Fire,cell,1);if(ch){if(const ZoneMob*zone=dynamic_cast<const ZoneMob*>(ch)){if(zone->kind()==ZoneMobKind::Elemental)break;}ch->damage(Random::Int(1,8+power()*power()),this);if(Burning*b=Buff::affect<Burning>(*ch))b->reignite(*ch);}break;}
    case WandKind::Poison:if(ch){if(Poison*p=Buff::affect<Poison>(*ch))p->set(Poison::durationFactor(*ch)*(5+power()));}break;
    case WandKind::Slowness:if(ch)Buff::prolong<Slow>(*ch,Slow::Duration/3.0f+power());break;
    case WandKind::Amok:if(ch){if(ch==&hero)Buff::prolong<Vertigo>(*ch,Vertigo::Duration);else Buff::prolong<Amok>(*ch,3.0f+power());}break;
    case WandKind::Blink:{int index=std::min(Ballistica::distance()-1,power()+3);int destination=trace[static_cast<std::size_t>(std::max(1,index))];if(d.scheduler().findChar(destination)&&Ballistica::distance()>1)destination=trace[static_cast<std::size_t>(Ballistica::distance()-2)];d.relocateChar(hero,destination);break;}
    case WandKind::Teleportation:{
        // Teleportation targets a creature.  A shot at an empty cell is a
        // harmless miss; it must not teleport the wand user.
        if(!ch || ch==&hero)break;
        for(int attempt=0;attempt<10;++attempt){
            const int p=level.randomDestination();
            if(p>=0&&d.relocateChar(*ch,p))break;
        }
        break;
    }
    case WandKind::Lightning:{if(!ch)break;std::unordered_set<Char*>affected;auto arc=[&](auto&& self,Char&target,int damage)->void{if(damage<1||affected.count(&target))return;affected.insert(&target);const int waterDamage=level.water(target.pos)&&!target.flying?damage*2:damage;const int dealtDamage=target.resistantTo(typeid(CatalogWand))?std::max(1,waterDamage/2):waterDamage;target.damage(dealtDamage,this);std::vector<Char*>next;for(int offset:Level::Neighbours8)if(Char*n=d.scheduler().findChar(target.pos+offset))if(!affected.count(n))next.push_back(n);if(!next.empty())self(self,*next[static_cast<std::size_t>(Random::Int(static_cast<int>(next.size())))],Random::Int(std::max(1,damage/2),damage));};arc(arc,*ch,Random::Int(5+power()/2,10+power()));break;}
    case WandKind::Reach:{const int reach=std::min(Ballistica::distance(),power()+4);for(int i=1;i<reach;++i){const int p=trace[static_cast<std::size_t>(i)];const int before=level.cell(p);if(Char*other=d.scheduler().findChar(p)){d.scheduler().freeCell(hero.pos);d.scheduler().freeCell(other->pos);std::swap(hero.pos,other->pos);d.scheduler().occupyCell(&hero);d.scheduler().occupyCell(other);level.press(hero.pos,&hero);level.press(other->pos,other);break;}if(Heap*heap=level.heapAt(p)){if(heap->type==Heap::Type::Heap){std::unique_ptr<Item>item=heap->pickUp();if(item){if(hero.belongings().backpack().canCollect(*item))hero.belongings().backpack().collect(std::move(item));else level.drop(std::move(item),hero.pos);level.removeEmptyHeap(p);}}else heap->type=Heap::Type::Heap;break;}level.press(p,nullptr);if(before==Terrain::OPEN_DOOR)level.setRaw(p,Terrain::DOOR);}level.buildFlagMaps();d.observe();break;}
    case WandKind::Flock:{int center=cell;if(d.scheduler().findChar(center)&&Ballistica::distance()>2)center=trace[static_cast<std::size_t>(Ballistica::distance()-2)];std::array<bool,Level::Length>passable{};for(int i=0;i<Level::Length;++i)passable[static_cast<std::size_t>(i)]=(level.passable(i)||level.avoid(i))&&!d.scheduler().findChar(i);PathFinder::buildDistanceMap(center,passable,power()+2);int made=0;for(int distance=0;distance<=power()+2&&made<power()+2;++distance)for(int p=0;p<Level::Length&&made<power()+2;++p)if(PathFinder::distance()[static_cast<std::size_t>(p)]==distance&&!d.scheduler().findChar(p)){std::unique_ptr<ZoneMob>sheep(new ZoneMob(ZoneMobKind::Sheep));/* ZoneMob consumes one life point on its initial scheduler tick. */sheep->setSheepLife(power()+4+Random::Int(3));ZoneMob*rawSheep=sheep.get();rawSheep->pos=p;rawSheep->bind(&d);level.addMob(std::move(sheep));d.scheduler().add(rawSheep);level.mobPress(*rawSheep);++made;}break;}
    case WandKind::Disintegration:{
        // Ring of Power increases damage, but the original ray range depends
        // on the wand's own level only.
        const int limit=std::min(Ballistica::distance(),this->level()+4);std::vector<Char*>targets;for(int i=1;i<limit;++i){const int p=trace[static_cast<std::size_t>(i)];if(Char*target=d.scheduler().findChar(p))targets.push_back(target);const int terrain=level.cell(p);if(terrain==Terrain::DOOR||terrain==Terrain::SIGN)level.destroy(p);else if(terrain==Terrain::HIGH_GRASS)level.setRaw(p,Terrain::GRASS);}const int scaled=power()+static_cast<int>(targets.size());for(Char*target:targets){if(const ZoneMob*zone=dynamic_cast<const ZoneMob*>(target))if(zone->kind()==ZoneMobKind::Eye)continue;int damage=Random::NormalIntRange(scaled,8+scaled*scaled/3);if(const ZoneBoss*boss=dynamic_cast<const ZoneBoss*>(target))if(boss->kind()==ZoneBossKind::King)damage=std::max(1,damage/2);target->damage(damage,this);}level.buildFlagMaps();d.observe();break;}
    case WandKind::Avalanche:{
        // Java shortens the ballistic trace to 8 + power before the rockfall
        // is centred.  Without this cap, selecting a distant cell created an
        // avalanche anywhere on the map.
        const int reach=std::min(Ballistica::distance(),8+power());
        const int center=trace[static_cast<std::size_t>(std::max(1,reach-1))];
        const int radius=1+power()/3;std::array<bool,Level::Length>open{};for(int i=0;i<Level::Length;++i)open[static_cast<std::size_t>(i)]=!level.solidMap()[static_cast<std::size_t>(i)];PathFinder::buildDistanceMap(center,open,radius);for(int p=0;p<Level::Length;++p){const int distance=PathFinder::distance()[static_cast<std::size_t>(p)];if(distance<0||distance>radius)continue;if(Char*target=d.scheduler().findChar(p)){target->damage(Random::Int(2,6+(radius-distance)*2),this);if(target->isAlive()&&Random::Int(2+distance)==0)Buff::prolong<Paralysis>(*target,Random::IntRange(2,6));if(Mob*mob=dynamic_cast<Mob*>(target))level.mobPress(*mob);else level.press(p,target);}else level.press(p,nullptr);}break;}
    }
}
std::unique_ptr<Item> CatalogWand::clone()const{std::unique_ptr<CatalogWand>copy(new CatalogWand(kind_));copy->quantity(quantity());copy->level(level());copy->setCursed(cursed());copy->setCursedKnown(cursedKnown());if(levelKnown())copy->identify();copy->restoreDurability(durability());copy->restoreCharges(curCharges());return std::move(copy);}
}
