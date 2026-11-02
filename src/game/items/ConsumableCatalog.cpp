#include "game/items/ConsumableCatalog.h"
#include "game/items/ItemSpriteSheet.h"
namespace pdv {
CatalogSeed::CatalogSeed(SeedKind kind):kind_(kind){static const char*n[]={"firebloom seed","icecap seed","sorrowmoss seed","dreamweed seed","sungrass seed","earthroot seed","fadeleaf seed","rotberry seed"};name_=n[static_cast<int>(kind)];image_=ItemSpriteSheet::SEED_FIREBLOOM+static_cast<int>(kind);}
std::unique_ptr<Item> CatalogSeed::clone()const{return std::unique_ptr<Item>(new CatalogSeed(*this));}
MysteryMeat::MysteryMeat(){name_="mystery meat";image_=ItemSpriteSheet::MEAT;setEnergy(150.0f);}
std::unique_ptr<Item> MysteryMeat::clone()const{return std::unique_ptr<Item>(new MysteryMeat(*this));}
CatalogMisc::CatalogMisc(MiscKind kind):kind_(kind){stackable_=true;name_=kind==MiscKind::Bomb?"bomb":"honeypot";image_=kind==MiscKind::Bomb?ItemSpriteSheet::BOMB:ItemSpriteSheet::HONEYPOT;}
std::unique_ptr<Item> CatalogMisc::clone()const{return std::unique_ptr<Item>(new CatalogMisc(*this));}
}
