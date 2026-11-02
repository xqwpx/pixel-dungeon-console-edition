#pragma once
#include "game/items/potions/Potion.h"
namespace pdv { class PotionOfFrost : public Potion { public: static constexpr int Distance=2; PotionOfFrost(); void shatter(Dungeon&,int) override; std::unique_ptr<Item> clone() const override; }; }
