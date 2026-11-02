#pragma once
#include "game/actors/mobs/Mob.h"
namespace pdv { class Skeleton : public Mob { public: Skeleton(); int damageRoll() const override; int attackSkill(const Char&) const override; int dr() const override; void die(const void*) override; const char* description() const override;protected:float lootChance()const override{return 0.2f;}std::unique_ptr<Item>createLoot()const override; }; }
