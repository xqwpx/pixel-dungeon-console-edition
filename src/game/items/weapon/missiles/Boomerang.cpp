#include "game/items/weapon/missiles/Boomerang.h"
#include "game/items/ItemSpriteSheet.h"
namespace pdv {
Boomerang::Boomerang(){ name_="boomerang";image_=ItemSpriteSheet::BOOMERANG;strength_=10;stackable_=false; }
std::unique_ptr<Item> Boomerang::clone() const { return std::unique_ptr<Item>(new Boomerang(*this)); }
} // namespace pdv
