#pragma once
#include "game/actors/mobs/Mob.h"

namespace pdv {

class Hero;

class MirrorImage : public Mob {
public:
    MirrorImage();

    void duplicate(const Hero& hero);

    int attackSkill(const Char& target) const override;
    int damageRoll() const override;
    int attackProc(Char& enemy, int damage) override;

protected:
    Char* chooseEnemy() override;

private:
    int attack_ = 0;
    int damage_ = 1;
};

} // namespace pdv
