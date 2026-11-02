#include "game/items/potions/PotionOfMight.h"
#include "game/actors/Hero.h"
#include "game/items/ItemSpriteSheet.h"
#include "game/Progression.h"
namespace pdv { PotionOfMight::PotionOfMight(){name_="Potion of Might";image_=ItemSpriteSheet::POTION_INDIGO;} void PotionOfMight::apply(Dungeon&,Hero&h){setKnown();h.increaseStrength();h.HT+=5;h.HP+=5;Progression::instance().noteStrength(h.baseStrength());} std::unique_ptr<Item> PotionOfMight::clone() const{return std::unique_ptr<Item>(new PotionOfMight(*this));} }
