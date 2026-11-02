#pragma once
#include <memory>
#include "game/actors/mobs/Mob.h"
#include "game/items/Item.h"
namespace pdv { class Thief : public Mob { public: explicit Thief(bool bandit=false); int damageRoll() const override; int attackSkill(const Char&) const override; int dr() const override; int attackProc(Char&,int) override;int defenseProc(Char&,int)override; void die(const void*) override; const Item* stolenItem()const{return stolen_.get();}void restoreStolen(std::unique_ptr<Item>item){stolen_=std::move(item);}bool isBandit()const{return bandit_;} const char* description()const override; protected: float attackDelay()const override{return 0.5f;}float lootChance()const override{return 0.01f;}std::unique_ptr<Item>createLoot()const override;bool actFleeing(bool enemyInFOV) override; private: bool bandit_=false;std::unique_ptr<Item> stolen_; }; }
