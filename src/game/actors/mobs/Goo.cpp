#include "game/actors/mobs/Goo.h"

#include "game/Dungeon.h"
#include "game/actors/buffs/Buff.h"
#include "game/actors/buffs/Ooze.h"
#include "game/actors/blobs/ToxicGas.h"
#include "game/items/keys/SkeletonKey.h"
#include "game/items/SpecialItems.h"
#include "game/levels/RegularLevel.h"
#include "game/mechanics/Ballistica.h"
#include "game/utils/Random.h"

namespace pdv {

Goo::Goo() {
    name = "Goo";
    HP = HT = 80;
    defenseSkill_ = 12;
    exp_ = 10;
    maxLvl_ = 30;
}

bool Goo::act() {
    if (dungeon_ && dungeon_->level().water(pos) && HP < HT) ++HP;
    return Mob::act();
}

int Goo::damageRoll() const {
    return pumpedUp_
        ? Random::NormalIntRange(5, 30)
        : Random::NormalIntRange(2, 12);
}

int Goo::attackSkill(const Char&) const { return pumpedUp_ && !jumped_ ? 30 : 15; }
int Goo::dr() const { return 2; }
std::unique_ptr<Item>Goo::createLoot()const{return std::unique_ptr<Item>(new SpecialItem(SpecialKind::LloydsBeacon));}

void Goo::die(const void* cause) {
    Mob::die(cause);
    if (dungeon_) {
        if (RegularLevel* level = dynamic_cast<RegularLevel*>(&dungeon_->level())) {
            level->unsealBossEntrance();
        }
        dungeon_->level().drop(
            std::unique_ptr<Item>(new SkeletonKey(dungeon_->depth())),
            pos);
    }
}

bool Goo::canAttack(const Char& enemy) const {
    return pumpedUp_ ? distance(enemy) <= 2 : Mob::canAttack(enemy);
}

int Goo::attackProc(Char& enemy, int damage) {
    if (Random::Int(3) == 0) Buff::affect<Ooze>(enemy);
    return damage;
}

bool Goo::immuneTo(const std::type_info& type) const {
    return Mob::immuneTo(type);
}

bool Goo::resistantTo(const std::type_info& type) const {
    return type == typeid(ToxicGas);
}

bool Goo::doAttack(Char& enemy) {
    if (!pumpedUp_ && Random::Int(3) == 0) {
        pumpedUp_ = true;
        jumped_ = false;
        spend(2.0f);
        next();
        return true;
    }
    if (pumpedUp_ && !Level::adjacent(pos, enemy.pos) && dungeon_) {
        const int impact = Ballistica::cast(*dungeon_, pos, enemy.pos, false, true);
        if (impact == enemy.pos && Ballistica::distance() >= 2) {
            const int destination = Ballistica::trace()[
                static_cast<std::size_t>(Ballistica::distance() - 2)];
            if (destination != pos && moveTo(destination, dungeon_->level())) {
                dungeon_->level().mobPress(*this);
                if (RegularLevel* level = dynamic_cast<RegularLevel*>(&dungeon_->level())) {
                    level->sealBossEntrance();
                }
                jumped_ = true;
            }
        } else {
            pumpedUp_ = false;
            spend(TICK);
            next();
            return true;
        }
    }
    const bool result = Mob::doAttack(enemy);
    pumpedUp_ = false;
    jumped_ = false;
    return result;
}

bool Goo::getCloser(int target) {
    pumpedUp_ = false;
    jumped_ = false;
    const bool moved = Mob::getCloser(target);
    if (moved && dungeon_) {
        if (RegularLevel* level = dynamic_cast<RegularLevel*>(&dungeon_->level())) {
            level->sealBossEntrance();
        }
    }
    return moved;
}

const char* Goo::description() const {
    return "A conglomerate of sewer substances that gained a rudimentary will.";
}

} // namespace pdv
