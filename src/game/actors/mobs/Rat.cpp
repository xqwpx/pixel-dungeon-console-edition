#include "game/actors/mobs/Rat.h"

#include "game/utils/Random.h"

namespace pdv {

Rat::Rat() {
    name = "marsupial rat";

    HP = HT = 8;

    defenseSkill_ = 3;
    maxLvl_ = 5;
}

int Rat::damageRoll() const {
    return Random::NormalIntRange(1, 5);
}

int Rat::attackSkill(const Char&) const {
    return 8;
}

int Rat::dr() const {
    return 1;
}

const char* Rat::description() const {
    return
        "Marsupial rats are aggressive, but rather weak denizens "
        "of the sewers. They can be dangerous only in big numbers.";
}

} // namespace pdv
