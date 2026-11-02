#pragma once
#include "game/items/armor/Armor.h"
#include "game/items/rings/Ring.h"
#include "game/items/wands/Wand.h"
#include "game/items/weapon/melee/MeleeWeapon.h"
#include "game/items/weapon/missiles/MissileWeapon.h"

namespace pdv {
enum class MeleeKind{Quarterstaff,Spear,Mace,Sword,Longsword,BattleAxe,WarHammer,Glaive};
class CatalogMelee:public MeleeWeapon{public:explicit CatalogMelee(MeleeKind kind);MeleeKind kind()const{return kind_;}bool sameKind(const Item&o)const override{const CatalogMelee*v=dynamic_cast<const CatalogMelee*>(&o);return v&&v->kind_==kind_;}std::unique_ptr<Item> clone()const override;private:MeleeKind kind_;};
enum class MissileKind{Javelin,IncendiaryDart,CurareDart,Shuriken,Tomahawk};
class CatalogMissile:public MissileWeapon{public:explicit CatalogMissile(MissileKind kind);MissileKind kind()const{return kind_;}bool sameKind(const Item&o)const override{const CatalogMissile*v=dynamic_cast<const CatalogMissile*>(&o);return v&&v->kind_==kind_;}int min()const override{return min_;}int max()const override{return max_;}float speedFactor(const Hero&)const override;void proc(Char&,Char&,int)override;bool onMiss(Dungeon&,int)override;int price()const override;std::unique_ptr<Item> clone()const override;private:MissileKind kind_;int min_=1,max_=4;};
enum class ArmorKind{Leather=2,Mail=3,Scale=4,Plate=5};
class CatalogArmor:public Armor{public:explicit CatalogArmor(ArmorKind kind);ArmorKind kind()const{return kind_;}bool sameKind(const Item&o)const override{const CatalogArmor*v=dynamic_cast<const CatalogArmor*>(&o);return v&&v->kind_==kind_;}std::unique_ptr<Item> clone()const override;private:ArmorKind kind_;};
enum class RingKind{Mending,Detection,Power,Herbalism,Accuracy,Evasion,Satiety,Haste,Elements,Haggler,Thorns};
class CatalogRing:public Ring{public:explicit CatalogRing(RingKind kind);RingKind kind()const{return kind_;}int statusIndex()const override{return static_cast<int>(kind_);}bool sameKind(const Item&o)const override{const CatalogRing*v=dynamic_cast<const CatalogRing*>(&o);return v&&v->kind_==kind_;}int accuracyBonus()const override;int evasionBonus()const override;int hasteBonus()const override;int satietyBonus()const override;int mendingBonus()const override;int powerBonus()const override;int detectionBonus()const override;bool detectsSecrets()const override;int herbalismBonus()const override;int elementsBonus()const override;bool haggling()const override;bool thorns()const override;std::unique_ptr<Item> clone()const override;private:RingKind kind_;};
enum class WandKind{Teleportation,Slowness,Firebolt,Regrowth,Poison,Blink,Lightning,Amok,Reach,Flock,Disintegration,Avalanche};
class CatalogWand:public Wand{public:explicit CatalogWand(WandKind kind);WandKind kind()const{return kind_;}int statusIndex()const override{return static_cast<int>(kind_);}bool sameKind(const Item&o)const override{const CatalogWand*v=dynamic_cast<const CatalogWand*>(&o);return v&&v->kind_==kind_;}void onZap(Dungeon&,Hero&,int)override;std::unique_ptr<Item> clone()const override;private:WandKind kind_;};
}
