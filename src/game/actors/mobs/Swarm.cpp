#include "game/actors/mobs/Swarm.h"

#include "game/Dungeon.h"
#include "game/actors/buffs/Buff.h"
#include "game/actors/buffs/Burning.h"
#include "game/actors/buffs/Poison.h"
#include "game/items/ConsumableCatalog.h"
#include "game/items/potions/PotionOfHealing.h"
#include "game/utils/Random.h"

#include <vector>

namespace pdv {

Swarm::Swarm() {
    name = "swarm of flies";
    HP = HT = 80;
    defenseSkill_ = 5;
    maxLvl_ = 10;
    flying = true;
}

int Swarm::damageRoll() const { return Random::NormalIntRange(1, 4); }
int Swarm::attackSkill(const Char&) const { return 12; }

int Swarm::defenseProc(Char&, int damage) {
    if (!dungeon_ || HP < damage + 2) return damage;
    std::vector<int> cells;
    for (int offset : Level::Neighbours4) {
        const int cell = pos + offset;
        if (dungeon_->level().inBoundsCell(cell) &&
            dungeon_->level().passable(cell) &&
            dungeon_->scheduler().findChar(cell) == nullptr) cells.push_back(cell);
    }
    if (cells.empty()) return damage;

    std::unique_ptr<Swarm> clone(new Swarm());
    clone->generation_ = generation_ + 1;
    if (findBuff(typeid(Burning))) {
        if (Burning* burning = Buff::affect<Burning>(*clone)) burning->reignite(*clone);
    }
    if (findBuff(typeid(Poison))) {
        if (Poison* poison = Buff::affect<Poison>(*clone)) poison->set(2.0f);
    }
    clone->HP = (HP - damage) / 2;
    HP -= clone->HP;
    clone->pos = cells[static_cast<std::size_t>(Random::Int(static_cast<int>(cells.size())))];
    clone->setState(State::Hunting);
    clone->bind(dungeon_);
    Swarm* raw = clone.get();
    dungeon_->level().addMob(std::move(clone));
    dungeon_->scheduler().addDelayed(raw, 1.0f);
    return damage;
}

const char* Swarm::description() const {
    return "A deadly swarm that splits into two smaller swarms when struck.";
}

float Swarm::lootChance() const {
    return 1.0f / static_cast<float>(5 * (generation_ + 1));
}

std::unique_ptr<Item> Swarm::createLoot() const {
    return std::unique_ptr<Item>(new PotionOfHealing());
}

} // namespace pdv
