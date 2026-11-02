#pragma once
#include "game/items/potions/Potion.h"
namespace pdv { class PotionOfExperience : public Potion { public: PotionOfExperience(); void apply(Dungeon&,Hero&) override; std::unique_ptr<Item> clone() const override; }; }
