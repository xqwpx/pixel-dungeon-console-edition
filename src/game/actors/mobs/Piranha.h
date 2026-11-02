#pragma once
#include "game/actors/mobs/Mob.h"
namespace pdv { class Piranha : public Mob { public: explicit Piranha(int depth); bool act() override;void die(const void*)override; int damageRoll()const override; int attackSkill(const Char&)const override; int dr()const override;bool immuneTo(const std::type_info&)const override; const char* description()const override; protected: bool getCloser(int)override;bool getFurther(int)override; private:int depth_=1; }; }
