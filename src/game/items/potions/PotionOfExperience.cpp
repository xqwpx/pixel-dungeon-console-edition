#include "game/items/potions/PotionOfExperience.h"
#include "game/actors/Hero.h"
#include "game/items/ItemSpriteSheet.h"
namespace pdv { PotionOfExperience::PotionOfExperience(){name_="Potion of Experience";image_=ItemSpriteSheet::POTION_CRIMSON;} void PotionOfExperience::apply(Dungeon&,Hero&h){setKnown();h.earnExp(h.maxExp()-h.exp());} std::unique_ptr<Item> PotionOfExperience::clone() const{return std::unique_ptr<Item>(new PotionOfExperience(*this));} }
