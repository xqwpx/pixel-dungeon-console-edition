#include "game/items/bags/ScrollHolder.h"
#include "game/items/scrolls/Scroll.h"
#include "game/items/ItemSpriteSheet.h"
namespace pdv {
ScrollHolder::ScrollHolder() : Bag(12, "scroll holder") { image_ = ItemSpriteSheet::HOLDER; }
bool ScrollHolder::grab(const Item& item) const { return dynamic_cast<const Scroll*>(&item) != nullptr; }
std::unique_ptr<Item> ScrollHolder::clone() const { return std::unique_ptr<Item>(new ScrollHolder()); }
}
