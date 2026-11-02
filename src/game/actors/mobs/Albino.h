#pragma once

#include "game/actors/mobs/Rat.h"

namespace pdv {

class Albino : public Rat {
public:
    Albino();
    int attackProc(Char& enemy, int damage) override;
};

} // namespace pdv
