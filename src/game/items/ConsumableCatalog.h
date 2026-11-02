#pragma once
#include "game/items/Item.h"
#include "game/items/food/Food.h"
#include "game/items/seeds/Seed.h"
namespace pdv {
enum class SeedKind{Firebloom,Icecap,Sorrowmoss,Dreamweed,Sungrass,Earthroot,Fadeleaf,Rotberry};
class CatalogSeed:public Seed{public:explicit CatalogSeed(SeedKind kind);SeedKind kind()const{return kind_;}bool sameKind(const Item&o)const override{const CatalogSeed*v=dynamic_cast<const CatalogSeed*>(&o);return v&&v->kind_==kind_;}std::unique_ptr<Item> clone()const override;private:SeedKind kind_;};
class MysteryMeat:public Food{public:MysteryMeat();int price()const override{return 5*quantity();}std::unique_ptr<Item> clone()const override;};
enum class MiscKind{Bomb,Honeypot};
class CatalogMisc:public Item{public:explicit CatalogMisc(MiscKind kind);MiscKind kind()const{return kind_;}bool sameKind(const Item&o)const override{const CatalogMisc*v=dynamic_cast<const CatalogMisc*>(&o);return v&&v->kind_==kind_;}std::unique_ptr<Item> clone()const override;bool isUpgradable()const override{return false;}bool isIdentified()const override{return true;}int price()const override{return (kind_==MiscKind::Bomb?10:50)*quantity();}std::vector<ItemAction> actions(const Hero&)const override{return kind_==MiscKind::Honeypot?std::vector<ItemAction>{ItemAction::Throw,ItemAction::Shatter,ItemAction::Drop}:std::vector<ItemAction>{ItemAction::Throw,ItemAction::Drop};}private:MiscKind kind_;};
}
