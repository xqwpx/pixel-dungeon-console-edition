#include "game/items/weapon/missiles/Dart.h"
#include "game/items/ItemSpriteSheet.h"
namespace pdv {
Dart::Dart(int number){ name_="dart";image_=ItemSpriteSheet::DART;quantity_=number; }
std::unique_ptr<Item> Dart::clone() const { return std::unique_ptr<Item>(new Dart(*this)); }
} // namespace pdv
