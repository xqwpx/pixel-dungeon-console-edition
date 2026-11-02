#include "game/actors/mobs/Crab.h"

#include "game/utils/Random.h"
#include "game/items/ConsumableCatalog.h"

namespace pdv {

Crab::Crab() {
    name = "sewer crab";
    HP = HT = 15;
    defenseSkill_ = 5;
    baseSpeed_ = 1.0f;
    exp_ = 3;
    maxLvl_ = 9;
}

int Crab::damageRoll() const { return Random::NormalIntRange(3, 6); }
int Crab::attackSkill(const Char&) const { return 12; }
int Crab::dr() const { return 4; }
std::unique_ptr<Item>Crab::createLoot()const{return std::unique_ptr<Item>(new MysteryMeat());}
const char* Crab::description() const {
    return "A fast, heavily armoured sewer crab.";
}

} // namespace pdv
