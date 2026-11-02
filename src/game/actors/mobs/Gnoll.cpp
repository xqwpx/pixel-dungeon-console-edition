#include "game/actors/mobs/Gnoll.h"

#include "game/utils/Random.h"
#include "game/Dungeon.h"
#include "game/items/Gold.h"

namespace pdv {

Gnoll::Gnoll() {
    name = "gnoll scout";
    HP = HT = 12;
    defenseSkill_ = 4;
    exp_ = 2;
    maxLvl_ = 8;
}

int Gnoll::damageRoll() const { return Random::NormalIntRange(2, 5); }
int Gnoll::attackSkill(const Char&) const { return 11; }
int Gnoll::dr() const { return 2; }
std::unique_ptr<Item>Gnoll::createLoot()const{std::unique_ptr<Gold>gold(new Gold());gold->random(dungeon_?dungeon_->depth():1);return std::unique_ptr<Item>(gold.release());}
const char* Gnoll::description() const {
    return "A nimble gnoll scout roaming the upper sewers.";
}

} // namespace pdv
