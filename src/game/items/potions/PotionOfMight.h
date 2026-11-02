#pragma once
#include "game/items/potions/PotionOfStrength.h"
namespace pdv { class PotionOfMight : public PotionOfStrength { public: PotionOfMight(); void apply(Dungeon&,Hero&) override; std::unique_ptr<Item> clone() const override; }; }
