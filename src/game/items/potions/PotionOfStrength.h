#pragma once
#include "game/items/potions/Potion.h"
namespace pdv { class PotionOfStrength : public Potion { public: PotionOfStrength(); void apply(Dungeon&,Hero&) override; std::unique_ptr<Item> clone() const override; }; }
