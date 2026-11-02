#include "game/items/bags/Keyring.h"
#include "game/items/ItemSpriteSheet.h"
#include "game/items/keys/Key.h"
namespace pdv {
Keyring::Keyring() : Bag(12, "key ring") {image_=ItemSpriteSheet::KEYRING;levelKnown_=true;cursedKnown_=true;}
bool Keyring::grab(const Item& item) const { return dynamic_cast<const Key*>(&item)!=nullptr; }
std::unique_ptr<Item> Keyring::clone() const { return std::unique_ptr<Item>(new Keyring()); }
} // namespace pdv
