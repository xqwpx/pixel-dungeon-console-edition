#include "game/items/bags/SeedPouch.h"
#include "game/items/seeds/Seed.h"
#include "game/items/ItemSpriteSheet.h"
namespace pdv {
SeedPouch::SeedPouch() : Bag(8, "seed pouch") { image_ = ItemSpriteSheet::POUCH; }
bool SeedPouch::grab(const Item& item) const { return dynamic_cast<const Seed*>(&item) != nullptr; }
std::unique_ptr<Item> SeedPouch::clone() const { return std::unique_ptr<Item>(new SeedPouch()); }
}
