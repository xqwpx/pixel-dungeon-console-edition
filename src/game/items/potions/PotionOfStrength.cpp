#include "game/items/potions/PotionOfStrength.h"
#include "game/actors/Hero.h"
#include "game/items/ItemSpriteSheet.h"
#include "game/Progression.h"
namespace pdv { PotionOfStrength::PotionOfStrength(){name_="Potion of Strength";image_=ItemSpriteSheet::POTION_CHARCOAL;} void PotionOfStrength::apply(Dungeon&,Hero&h){setKnown();h.increaseStrength();Progression::instance().noteStrength(h.baseStrength());} std::unique_ptr<Item> PotionOfStrength::clone() const{return std::unique_ptr<Item>(new PotionOfStrength(*this));} }
