#include "game/actors/mobs/ZoneMob.h"
#include "game/Dungeon.h"
#include "game/Progression.h"
#include "game/actors/buffs/Burning.h"
#include "game/actors/buffs/Charm.h"
#include "game/actors/buffs/Poison.h"
#include "game/actors/buffs/Roots.h"
#include "game/actors/buffs/Weakness.h"
#include "game/actors/buffs/Cripple.h"
#include "game/actors/buffs/Paralysis.h"
#include "game/actors/buffs/Ooze.h"
#include "game/actors/buffs/Amok.h"
#include "game/actors/buffs/Terror.h"
#include "game/actors/buffs/Vertigo.h"
#include "game/actors/buffs/Sleep.h"
#include "game/actors/buffs/Frost.h"
#include "game/actors/buffs/Light.h"
#include "game/actors/blobs/Blob.h"
#include "game/actors/blobs/ToxicGas.h"
#include "game/pathfinding/PathFinder.h"
#include "game/items/keys/SkeletonKey.h"
#include "game/items/SpecialItems.h"
#include "game/items/KindOfWeapon.h"
#include "game/items/weapon/melee/Knuckles.h"
#include "game/items/rings/Ring.h"
#include "game/items/EquipmentCatalog.h"
#include "game/items/Generator.h"
#include "game/items/Gold.h"
#include "game/items/food/Food.h"
#include "game/items/ConsumableCatalog.h"
#include "game/items/potions/PotionOfHealing.h"
#include "game/items/potions/PotionOfLiquidFlame.h"
#include "game/items/scrolls/Scroll.h"
#include "game/levels/RegularLevel.h"
#include "game/levels/Terrain.h"
#include "game/utils/Random.h"
#include "game/mechanics/Ballistica.h"
#include "game/levels/traps/LightningTrap.h"
#include <algorithm>
#include <stdlib.h>
namespace pdv {
ZoneMob::ZoneMob(ZoneMobKind kind) : kind_(kind) {
    switch (kind) {
    case ZoneMobKind::Shaman: name="gnoll shaman"; HP=HT=18; defenseSkill_=8; exp_=6; maxLvl_=14; minDamage_=2; maxDamage_=6; skill_=11; dr_=4; ranged_=true; break;
    case ZoneMobKind::Bat: name="vampire bat"; HP=HT=18; defenseSkill_=7; baseSpeed_=1.0f; exp_=7; maxLvl_=15; minDamage_=5; maxDamage_=10; skill_=13; dr_=1; flying=true; break;
    case ZoneMobKind::Brute:
    case ZoneMobKind::Shielded:
        // These were tuned like late-game enemies: high evasion combined with
        // 8–10 armour made early weapons deal zero damage most of the time.
        // Keep the shielded variant tougher, but make both reliably hittable.
        name=kind==ZoneMobKind::Shielded?"shielded brute":"gnoll brute";
        HP=HT=kind==ZoneMobKind::Shielded?36:32;
        defenseSkill_=kind==ZoneMobKind::Shielded?12:10;
        exp_=8; maxLvl_=15;
        minDamage_=kind==ZoneMobKind::Shielded?7:6;
        maxDamage_=kind==ZoneMobKind::Shielded?15:13;
        skill_=kind==ZoneMobKind::Shielded?16:15;
        dr_=kind==ZoneMobKind::Shielded?5:3;
        break;
    case ZoneMobKind::Spinner: name="cave spinner"; HP=HT=50; defenseSkill_=14; exp_=9; maxLvl_=16; minDamage_=12; maxDamage_=16; skill_=20; dr_=6; break;
    case ZoneMobKind::Elemental: name="fire elemental"; HP=HT=65; defenseSkill_=20; exp_=10; maxLvl_=20; minDamage_=16; maxDamage_=20; skill_=25; dr_=5; break;
    case ZoneMobKind::Monk:
    case ZoneMobKind::Senior: name=kind==ZoneMobKind::Senior?"senior monk":"dwarf monk"; HP=HT=70; defenseSkill_=30; exp_=11; maxLvl_=21; minDamage_=12; maxDamage_=16; skill_=30; dr_=2; break;
    case ZoneMobKind::Golem: name="golem"; HP=HT=85; defenseSkill_=18; exp_=12; maxLvl_=22; minDamage_=20; maxDamage_=40; skill_=28; dr_=12; break;
    case ZoneMobKind::Warlock: name="dwarf warlock"; HP=HT=70; defenseSkill_=18; exp_=11; maxLvl_=21; minDamage_=12; maxDamage_=20; skill_=25; dr_=8; ranged_=true; break;
    case ZoneMobKind::Succubus: name="succubus"; HP=HT=80; defenseSkill_=25; exp_=12; maxLvl_=25; minDamage_=15; maxDamage_=25; skill_=40; dr_=10; break;
    case ZoneMobKind::Eye: name="evil eye"; HP=HT=100; defenseSkill_=20; exp_=13; maxLvl_=25; minDamage_=18; maxDamage_=30; skill_=30; dr_=10; ranged_=true; flying=true; break;
    case ZoneMobKind::Scorpio:
    case ZoneMobKind::Acidic: name=kind==ZoneMobKind::Acidic?"acidic scorpio":"scorpio"; HP=HT=95; defenseSkill_=24; exp_=14; maxLvl_=25; minDamage_=20; maxDamage_=32; skill_=36; dr_=16; ranged_=true; break;
    case ZoneMobKind::Bandit: name="crazy bandit"; HP=HT=20; defenseSkill_=12; exp_=5; maxLvl_=10; minDamage_=2; maxDamage_=8; skill_=12; dr_=3; break;
    case ZoneMobKind::Undead: name="undead dwarf"; HP=HT=28; defenseSkill_=15; exp_=0; minDamage_=12; maxDamage_=16; skill_=16; dr_=5; state_=State::Wandering; break;
    case ZoneMobKind::RottingFist: name="rotting fist"; HP=HT=300; defenseSkill_=25; exp_=0; minDamage_=24; maxDamage_=36; skill_=36; dr_=15; state_=State::Wandering; break;
    case ZoneMobKind::BurningFist: name="burning fist"; HP=HT=200; defenseSkill_=25; exp_=0; minDamage_=20; maxDamage_=32; skill_=36; dr_=15; ranged_=true; state_=State::Wandering; break;
    case ZoneMobKind::Sheep: name="sheep"; HP=HT=1; defenseSkill_=0; exp_=0; state_=State::Passive; hostile_=false; life_=5; break;
    case ZoneMobKind::Larva: name="god's larva"; HP=HT=25; defenseSkill_=20; exp_=0; minDamage_=15; maxDamage_=20; skill_=30; dr_=8; state_=State::Hunting; break;
    }
}
bool ZoneMob::act() {
    if (kind_ == ZoneMobKind::Elemental) flying = true;

    if (kind_ == ZoneMobKind::Sheep) {
        if (--life_ <= 0) {
            destroy();
        } else {
            spend(TICK);
            next();
        }
        return true;
    }

    if (dungeon_) {
        if (kind_ == ZoneMobKind::RottingFist && dungeon_->level().water(pos) && HP < HT) HP += 4;
        if (kind_ == ZoneMobKind::BurningFist) {
            for (int offset : Level::Neighbours9) {
                const int p = pos + offset;
                if (dungeon_->level().inBoundsCell(p)) dungeon_->level().seedBlob(BlobKind::Fire, p, 2);
            }
        }
    }

    const bool result = Mob::act();
    if (kind_ == ZoneMobKind::Spinner && state_ == State::Fleeing && !hasBuff(typeid(Terror)) && enemy_ &&
        enemySeen_ && !enemy_->hasBuff(typeid(Poison))) {
        state_ = State::Hunting;
    }
    return result;
}
float ZoneMob::attackDelay() const {
    if (kind_ == ZoneMobKind::Monk || kind_ == ZoneMobKind::Senior) return 0.5f;
    if (kind_ == ZoneMobKind::Golem) return 1.5f;
    return 1.0f;
}
bool ZoneMob::getCloser(int target) {
    if ((kind_ == ZoneMobKind::Scorpio || kind_ == ZoneMobKind::Acidic) && state_ == State::Hunting) {
        return enemySeen_ && getFurther(target);
    }

    if (kind_ == ZoneMobKind::Succubus && dungeon_ && dungeon_->level().inBoundsCell(target) &&
        dungeon_->level().fieldOfView()[static_cast<std::size_t>(target)] && Level::distance(pos, target) > 2 &&
        blinkDelay_ <= 0) {
        int cell = Ballistica::cast(*dungeon_, pos, target, true, true);
        if (dungeon_->scheduler().findChar(cell) && Ballistica::distance() > 1) {
            cell = Ballistica::trace()[static_cast<std::size_t>(Ballistica::distance() - 2)];
        }
        if (cell != pos && dungeon_->relocateChar(*this, cell)) {
            blinkDelay_ = 5;
            spend(-TICK / speed());
            return true;
        }
    }

    if (kind_ == ZoneMobKind::Succubus) --blinkDelay_;
    return Mob::getCloser(target);
}
bool ZoneMob::immuneTo(const std::type_info& type) const {
    if ((kind_ == ZoneMobKind::Monk || kind_ == ZoneMobKind::Senior) &&
        (type == typeid(Amok) || type == typeid(Terror))) return true;
    if ((kind_ == ZoneMobKind::Brute || kind_ == ZoneMobKind::Shielded || kind_ == ZoneMobKind::Eye) &&
        type == typeid(Terror)) return true;
    if (kind_ == ZoneMobKind::Spinner && type == typeid(Roots)) return true;
    if (kind_ == ZoneMobKind::Golem &&
        (type == typeid(Amok) || type == typeid(Terror) || type == typeid(Sleep))) return true;
    if (kind_ == ZoneMobKind::Succubus && type == typeid(Sleep)) return true;
    if (kind_ == ZoneMobKind::Undead && type == typeid(Paralysis)) return true;
    if (kind_ == ZoneMobKind::RottingFist &&
        (type == typeid(Amok) || type == typeid(Sleep) || type == typeid(Terror) || type == typeid(Poison) ||
         type == typeid(Vertigo) || type == typeid(ToxicGas))) return true;
    if (kind_ == ZoneMobKind::BurningFist &&
        (type == typeid(Amok) || type == typeid(Sleep) || type == typeid(Terror) || type == typeid(Burning) ||
         type == typeid(ToxicGas))) return true;
    return Mob::immuneTo(type);
}

bool ZoneMob::resistantTo(const std::type_info& type) const {
    if (type == typeid(Poison)) {
        return kind_ == ZoneMobKind::Spinner || kind_ == ZoneMobKind::Scorpio || kind_ == ZoneMobKind::Acidic;
    }
    return kind_ == ZoneMobKind::Shaman && (type == typeid(CatalogWand) || type == typeid(LightningTrap));
}
void ZoneMob::onBuffAdded(Buff& buff) {
    if (kind_ == ZoneMobKind::Elemental) {
        if (dynamic_cast<Burning*>(&buff)) {
            if (HP < HT) ++HP;
            buff.detach();
            return;
        }
        if (dynamic_cast<Frost*>(&buff)) damage(Random::NormalIntRange(1, HT * 2 / 3), &buff);
    }

    Mob::onBuffAdded(buff);
}
void ZoneMob::die(const void* cause) {
    if (dungeon_) {
        const int quest = dungeon_->questState(3);
        if ((quest == 401 && (kind_ == ZoneMobKind::Monk || kind_ == ZoneMobKind::Senior)) ||
            (quest == 402 && kind_ == ZoneMobKind::Golem)) {
            dungeon_->level().drop(std::unique_ptr<Item>(new SpecialItem(SpecialKind::DwarfToken)), pos);
        }
        if (kind_ == ZoneMobKind::Scorpio || kind_ == ZoneMobKind::Acidic) {
            if (Random::Int(8) == 0) {
                dungeon_->level().drop(std::unique_ptr<Item>(new PotionOfHealing()), pos);
            } else if (Random::Int(6) == 0) {
                dungeon_->level().drop(std::unique_ptr<Item>(new MysteryMeat()), pos);
            }
        }
    }

    Mob::die(cause);
}
float ZoneMob::lootChance() const {
    switch (kind_) {
    case ZoneMobKind::Bat: return 0.125f;
    case ZoneMobKind::Brute:
    case ZoneMobKind::Shielded:
    case ZoneMobKind::Eye: return 0.5f;
    case ZoneMobKind::Spinner: return 0.125f;
    case ZoneMobKind::Elemental: return 0.1f;
    case ZoneMobKind::Monk:
    case ZoneMobKind::Senior: return 0.083f;
    case ZoneMobKind::Warlock: return 0.83f;
    case ZoneMobKind::Succubus: return 0.05f;
    case ZoneMobKind::Shaman: return 0.33f;
    case ZoneMobKind::Bandit: return 0.01f;
    default: return 0.0f;
    }
}
std::unique_ptr<Item> ZoneMob::createLoot() const {
    switch (kind_) {
    case ZoneMobKind::Bat:
    case ZoneMobKind::Scorpio:
    case ZoneMobKind::Acidic:
        return std::unique_ptr<Item>(new PotionOfHealing());
    case ZoneMobKind::Brute:
    case ZoneMobKind::Shielded: {
        std::unique_ptr<Gold> gold(new Gold());
        gold->random(dungeon_ ? dungeon_->depth() : 1);
        return std::move(gold);
    }
    case ZoneMobKind::Spinner:
        return std::unique_ptr<Item>(new MysteryMeat());
    case ZoneMobKind::Elemental:
        return std::unique_ptr<Item>(new PotionOfLiquidFlame());
    case ZoneMobKind::Monk:
    case ZoneMobKind::Senior:
        return std::unique_ptr<Item>(new Food());
    case ZoneMobKind::Warlock:
        return Generator::random(Generator::Category::Potion, dungeon_ ? dungeon_->depth() : 1);
    case ZoneMobKind::Succubus:
        return std::unique_ptr<Item>(new Scroll(ScrollKind::Lullaby));
    case ZoneMobKind::Shaman:
        return Generator::random(Generator::Category::Scroll, dungeon_ ? dungeon_->depth() : 1);
    case ZoneMobKind::Eye:
        return std::unique_ptr<Item>(new SpecialItem(SpecialKind::Dewdrop));
    case ZoneMobKind::Bandit:
        return std::unique_ptr<Item>(new CatalogRing(RingKind::Haggler));
    default:
        return nullptr;
    }
}
void ZoneMob::damage(int amount, const void* source) {
    const bool enrage = (kind_ == ZoneMobKind::Brute || kind_ == ZoneMobKind::Shielded) &&
        HP >= HT / 4 && HP - amount < HT / 4;

    if (kind_ != ZoneMobKind::Sheep) Mob::damage(amount, source);
    if (enrage && isAlive()) spend(TICK);
}

int ZoneMob::damageRoll() const {
    if ((kind_ == ZoneMobKind::Brute || kind_ == ZoneMobKind::Shielded) && HP < HT / 4) {
        return Random::NormalIntRange(8, 20);
    }
    if (kind_ == ZoneMobKind::Senior) return Random::NormalIntRange(12, 20);
    return Random::NormalIntRange(minDamage_, maxDamage_);
}

int ZoneMob::attackSkill(const Char&) const {
    return skill_;
}

int ZoneMob::dr() const {
    return dr_;
}
bool ZoneMob::canAttack(const Char& e) const {
    if (!ranged_ || !dungeon_) return Mob::canAttack(e);
    if (kind_ == ZoneMobKind::Scorpio || kind_ == ZoneMobKind::Acidic) {
        return !Level::adjacent(pos, e.pos) && Ballistica::cast(*dungeon_, pos, e.pos, false, true) == e.pos;
    }
    if (kind_ == ZoneMobKind::Eye) {
        Ballistica::cast(*dungeon_, pos, e.pos, true, false);
        for (int i = 1; i < Ballistica::distance(); ++i) {
            if (Ballistica::trace()[static_cast<std::size_t>(i)] == e.pos) return true;
        }
        return false;
    }
    return Ballistica::cast(*dungeon_, pos, e.pos, false, true) == e.pos;
}
bool ZoneMob::doAttack(Char& e) {
    if (!ranged_ || (Level::adjacent(pos, e.pos) && kind_ != ZoneMobKind::Eye)) return Mob::doAttack(e);

    if (dungeon_) dungeon_->noteMobAttack(pos);

    if (kind_ == ZoneMobKind::Eye) {
        Ballistica::cast(*dungeon_, pos, e.pos, true, false);
        for (int i = 1; i < Ballistica::distance(); ++i) {
            if (Char* ch = dungeon_->scheduler().findChar(Ballistica::trace()[static_cast<std::size_t>(i)])) {
                if (Char::hit(*this, *ch, true)) ch->damage(Random::NormalIntRange(14, 20), this);
            }
        }
        spend(1.6f);
    } else if (Char::hit(*this, e, true)) {
        if (kind_ == ZoneMobKind::Shaman) {
            int damage = Random::Int(2, 12);
            if (dungeon_->level().water(e.pos) && !e.flying) damage = damage * 3 / 2;
            e.damage(damage, this);
        } else if (kind_ == ZoneMobKind::Warlock) {
            if (Random::Int(2) == 0) Buff::prolong<Weakness>(e, Weakness::Duration);
            e.damage(Random::Int(12, 18), this);
        } else {
            attack(e);
        }
        spend(kind_ == ZoneMobKind::Shaman ? 2.0f : 1.0f);
    } else {
        spend(kind_ == ZoneMobKind::Shaman ? 2.0f : 1.0f);
    }

    next();
    return e.isAlive();
}
int ZoneMob::attackProc(Char& enemy, int damage) {
    switch (kind_) {
    case ZoneMobKind::Bat:
        HP = std::min(HT, HP + std::max(0, damage / 3));
        break;
    case ZoneMobKind::Spinner:
        if (Random::Int(2) == 0) {
            if (Poison* poison = Buff::affect<Poison>(enemy)) poison->set(Random::IntRange(7, 9));
            state_ = State::Fleeing;
        }
        break;
    case ZoneMobKind::Scorpio:
        if (Random::Int(2) == 0) Buff::prolong<Cripple>(enemy, Cripple::Duration);
        break;
    case ZoneMobKind::Acidic:
        if (Random::Int(2) == 0) Buff::prolong<Cripple>(enemy, Cripple::Duration);
        break;
    case ZoneMobKind::Elemental:
        if (Random::Int(2) == 0) {
            if (Burning* fire = Buff::affect<Burning>(enemy)) fire->reignite(enemy);
        }
        break;
    case ZoneMobKind::Succubus:
        if (Random::Int(3) == 0) {
            Charm* charm = Buff::affect<Charm>(enemy, elementsDurationFactor(enemy) * Random::IntRange(3, 7));
            if (charm) charm->object = id();
        }
        break;
    case ZoneMobKind::Monk:
    case ZoneMobKind::Senior:
        if (kind_ == ZoneMobKind::Senior && Random::Int(10) == 0) Buff::prolong<Paralysis>(enemy, 1.1f);
        if (dungeon_ && &enemy == &dungeon_->hero() && Random::Int(6) == 0) {
            KindOfWeapon* weapon = dungeon_->hero().belongings().weapon();
            if (weapon && !dynamic_cast<Knuckles*>(weapon) && !weapon->cursed()) {
                std::unique_ptr<KindOfWeapon> dropped = dungeon_->hero().belongings().takeWeapon();
                const std::string weaponName = dropped->displayName();
                dungeon_->level().drop(std::move(dropped), enemy.pos);
                dungeon_->postMessage(name + " knocked the " + weaponName + " from your hands!");
            }
        }
        break;
    case ZoneMobKind::Undead:
        if (Random::Int(5) == 0) Buff::prolong<Paralysis>(enemy, 1);
        break;
    case ZoneMobKind::RottingFist:
        if (Random::Int(3) == 0) Buff::affect<Ooze>(enemy);
        break;
    default:
        break;
    }
    return damage;
}
int ZoneMob::defenseProc(Char& enemy, int damage) {
    if (kind_ == ZoneMobKind::Acidic) {
        const int reflected = Random::IntRange(0, damage);
        if (reflected > 0) enemy.damage(reflected, this);
    }
    return Mob::defenseProc(enemy, damage);
}

void ZoneMob::onMoved(int from) {
    if (kind_ == ZoneMobKind::Spinner && state_ == State::Fleeing && dungeon_) {
        dungeon_->level().seedBlob(BlobKind::Web, from, Random::IntRange(5, 7));
    }
}

const char* ZoneMob::description() const {
    return "A dangerous inhabitant of the deeper dungeon.";
}
ZoneBoss::ZoneBoss(ZoneBossKind kind) : kind_(kind) {
    switch (kind) {
    case ZoneBossKind::Tengu:
        name = "Tengu";
        HP = HT = 120; exp_ = 20; defenseSkill_ = 20; minDamage_ = 8; maxDamage_ = 15; skill_ = 20; dr_ = 5;
        break;
    case ZoneBossKind::DM300:
        name = "DM-300";
        HP = HT = 200; exp_ = 30; defenseSkill_ = 18; minDamage_ = 18; maxDamage_ = 24; skill_ = 28; dr_ = 10;
        break;
    case ZoneBossKind::King:
        name = "King of Dwarves";
        HP = HT = 300; exp_ = 40; defenseSkill_ = 25; minDamage_ = 20; maxDamage_ = 38; skill_ = 32; dr_ = 14;
        break;
    case ZoneBossKind::Yog:
        name = "Yog-Dzewa";
        HP = HT = 300; exp_ = 50; defenseSkill_ = 0; minDamage_ = 24; maxDamage_ = 36; skill_ = 36; dr_ = 15;
        state_ = State::Passive;
        break;
    }
}

void ZoneBoss::spawnMinion(ZoneMobKind kind, int cell) {
    if (!dungeon_ || !dungeon_->level().inBoundsCell(cell) || dungeon_->scheduler().findChar(cell) ||
        (!dungeon_->level().passable(cell) && !dungeon_->level().avoid(cell))) return;

    std::unique_ptr<ZoneMob> mob(new ZoneMob(kind));
    ZoneMob* raw = mob.get();
    raw->pos = cell;
    raw->bind(dungeon_);
    dungeon_->level().addMob(std::move(mob));
    dungeon_->scheduler().add(raw);
}
bool ZoneBoss::act() {
    if (!sealed_ && dungeon_ && dungeon_->level().fieldOfView()[static_cast<std::size_t>(pos)]) {
        if (RegularLevel* level = dynamic_cast<RegularLevel*>(&dungeon_->level())) level->sealBossEntrance();
        sealed_ = true;
    }
    if (dungeon_ && kind_ == ZoneBossKind::DM300) dungeon_->level().seedBlob(BlobKind::ToxicGas, pos, 30);
    if (sealed_ && dungeon_ && kind_ == ZoneBossKind::Yog && !summonsSpawned_) {
        std::vector<int> open;
        for (int offset : Level::Neighbours8) {
            const int cell = pos + offset;
            if (dungeon_->level().inBoundsCell(cell) && !dungeon_->scheduler().findChar(cell) &&
                (dungeon_->level().passable(cell) || dungeon_->level().avoid(cell))) open.push_back(cell);
        }
        if (open.size() >= 2) {
            const int first = Random::Int(static_cast<int>(open.size()));
            spawnMinion(ZoneMobKind::RottingFist, open[static_cast<std::size_t>(first)]);
            open.erase(open.begin() + first);
            spawnMinion(ZoneMobKind::BurningFist, open[static_cast<std::size_t>(Random::Int(static_cast<int>(open.size())))]);
            summonsSpawned_ = true;
        }
    }
    return Mob::act();
}
bool ZoneBoss::tenguJump() {
    if (!dungeon_) return false;
    Level& level = dungeon_->level();
    std::vector<int> visiblePassable;
    for (int cell = 0; cell < Level::Length; ++cell) {
        if (level.fieldOfView()[cell] && level.passable(cell)) visiblePassable.push_back(cell);
    }
    for (int i = 0; i < 4 && !visiblePassable.empty(); ++i) {
        const int trap = visiblePassable[static_cast<std::size_t>(Random::Int(static_cast<int>(visiblePassable.size())))];
        if (level.cell(trap) == Terrain::INACTIVE_TRAP) level.setRaw(trap, Terrain::POISON_TRAP);
    }
    level.buildFlagMaps();
    std::vector<int> destinations;
    for (int cell = 0; cell < Level::Length; ++cell) {
        // Tengu's arena is the lower part of floor 10.  He must not jump
        // through the sealed door into the rest of the level.
        if (cell / Level::Width >= 23 &&
            level.fieldOfView()[cell] && level.passable(cell) && !dungeon_->scheduler().findChar(cell) &&
            (!enemy_ || !Level::adjacent(cell, enemy_->pos))) destinations.push_back(cell);
    }
    if (destinations.empty()) return false;
    const int destination = destinations[static_cast<std::size_t>(Random::Int(static_cast<int>(destinations.size())))];
    jumpDelay_ = 5;
    dungeon_->relocateChar(*this, destination);
    spend(TICK);
    next();
    return true;
}
int ZoneBoss::kingPedestal() const {
    const int center = Level::Width / 2;
    return (2 + 15 / 2) * Level::Width + center + (nextPedestal_ ? -2 : 2);
}

bool ZoneBoss::kingCanSummon() const {
    if (kind_ != ZoneBossKind::King || !dungeon_) return false;
    int count = 0;
    for (const auto& mob : dungeon_->level().mobs()) {
        if (const ZoneMob* zone = dynamic_cast<const ZoneMob*>(mob.get())) {
            if (zone->kind() == ZoneMobKind::Undead && zone->isAlive()) ++count;
        }
    }
    const int limit = 1 + 5 * (HT - HP) / HT;
    Char* occupant = dungeon_->scheduler().findChar(kingPedestal());
    return count < limit && (!occupant || occupant == this);
}
bool ZoneBoss::getCloser(int target) {
    if (kind_ == ZoneBossKind::Tengu && dungeon_ && jumpDelay_ <= 0 &&
        dungeon_->level().inBoundsCell(target) && dungeon_->level().fieldOfView()[target] &&
        !Level::adjacent(pos, target)) return tenguJump();
    if (kind_ == ZoneBossKind::King && kingCanSummon()) return Mob::getCloser(kingPedestal());
    return Mob::getCloser(target);
}
void ZoneBoss::onMoved(int) {
    if (kind_ != ZoneBossKind::DM300 || !dungeon_) return;
    Level& level = dungeon_->level();
    if (level.cell(pos) == Terrain::INACTIVE_TRAP && HP < HT) HP += Random::IntRange(1, HT - HP);
    const int cell = pos + Level::Neighbours8[static_cast<std::size_t>(Random::Int(8))];
    if (!level.inBoundsCell(cell)) return;
    if (dungeon_->visible()[static_cast<std::size_t>(cell)] && level.cell(cell) == Terrain::EMPTY) {
        level.setRaw(cell, Terrain::EMPTY_DECO);
    }
    if (Char* ch = dungeon_->scheduler().findChar(cell)) {
        if (ch != this) Buff::prolong<Paralysis>(*ch, 2.0f);
    }
    level.buildFlagMaps();
}
int ZoneBoss::damageRoll() const { return Random::NormalIntRange(minDamage_, maxDamage_); }
int ZoneBoss::attackSkill(const Char&) const { return skill_; }
int ZoneBoss::dr() const { return dr_; }
float ZoneBoss::lootChance() const { return kind_ == ZoneBossKind::DM300 ? 0.333f : 0.0f; }

std::unique_ptr<Item> ZoneBoss::createLoot() const {
    if (kind_ != ZoneBossKind::DM300) return nullptr;
    std::unique_ptr<Item> loot(new CatalogRing(RingKind::Thorns));
    const int level = Random::Int(1, 3);
    if (Random::Float() < 0.3f) {
        loot->degrade(level);
        loot->setCursed(true);
    } else {
        loot->upgrade(level);
    }
    return loot;
}

bool ZoneBoss::canAttack(const Char& e) const {
    if (kind_ == ZoneBossKind::Tengu && dungeon_) return Ballistica::cast(*dungeon_, pos, e.pos, false, true) == e.pos;
    if (kind_ == ZoneBossKind::King && kingCanSummon()) return pos == kingPedestal();
    return Mob::canAttack(e);
}

bool ZoneBoss::doAttack(Char& e) {
    if (kind_ == ZoneBossKind::Tengu && --jumpDelay_ <= 0 && Level::adjacent(pos, e.pos) && tenguJump()) {
        return e.isAlive();
    }

    if (kind_ == ZoneBossKind::King && kingCanSummon() && pos == kingPedestal()) {
        int count = 0;
        for (const auto& mob : dungeon_->level().mobs()) {
            if (const ZoneMob* zone = dynamic_cast<const ZoneMob*>(mob.get())) {
                if (zone->kind() == ZoneMobKind::Undead && zone->isAlive()) ++count;
            }
        }

        const int needed = 1 + 5 * (HT - HP) / HT - count;
        std::array<bool, Level::Length> passable{};
        for (int cell = 0; cell < Level::Length; ++cell) {
            passable[static_cast<std::size_t>(cell)] = dungeon_->level().passable(cell);
        }
        for (Actor* actor : dungeon_->scheduler().all()) {
            if (const Char* ch = dynamic_cast<const Char*>(actor)) {
                if (dungeon_->level().inBoundsCell(ch->pos)) passable[static_cast<std::size_t>(ch->pos)] = false;
            }
        }

        PathFinder::buildDistanceMap(pos, passable, needed);
        const auto& distance = PathFinder::distance();
        std::array<bool, Level::Length> used{};
        int dist = 1;
        for (int i = 0; i < needed; ++i) {
            bool spawned = false;
            do {
                for (int cell = 0; cell < Level::Length; ++cell) {
                    if (!used[static_cast<std::size_t>(cell)] && distance[static_cast<std::size_t>(cell)] == dist) {
                        spawnMinion(ZoneMobKind::Undead, cell);
                        used[static_cast<std::size_t>(cell)] = true;
                        spawned = true;
                        break;
                    }
                }
                if (!spawned) ++dist;
            } while (!spawned && dist < needed);
        }

        nextPedestal_ = !nextPedestal_;
        spend(TICK);
        next();
        return e.isAlive();
    }

    if (kind_ == ZoneBossKind::King && dungeon_->scheduler().findChar(kingPedestal()) == &e) {
        nextPedestal_ = !nextPedestal_;
    }
    return Mob::doAttack(e);
}

int ZoneBoss::attackProc(Char&, int damage) {
    return damage;
}
int ZoneBoss::defenseProc(Char& enemy, int damage) {
    if (kind_ == ZoneBossKind::Yog && dungeon_) {
        std::vector<int> open;
        for (int offset : Level::Neighbours8) {
            const int p = pos + offset;
            if (dungeon_->level().inBoundsCell(p) && !dungeon_->scheduler().findChar(p) &&
                (dungeon_->level().passable(p) || dungeon_->level().avoid(p))) open.push_back(p);
        }
        if (!open.empty()) {
            spawnMinion(ZoneMobKind::Larva, open[static_cast<std::size_t>(Random::Int(static_cast<int>(open.size())))]);
        }
    }
    return Mob::defenseProc(enemy, damage);
}

void ZoneBoss::damage(int amount, const void* source) {
    if (kind_ == ZoneBossKind::Yog && dungeon_) {
        int fists = 0;
        for (const auto& mob : dungeon_->level().mobs()) {
            if (ZoneMob* zone = dynamic_cast<ZoneMob*>(mob.get())) {
                if ((zone->kind() == ZoneMobKind::RottingFist || zone->kind() == ZoneMobKind::BurningFist) && zone->isAlive()) {
                    zone->beckon(pos);
                    ++fists;
                }
            }
        }
        amount >>= fists;
    }
    Mob::damage(amount, source);
}

void ZoneBoss::beckon(int cell) {
    if (kind_ != ZoneBossKind::Yog) Mob::beckon(cell);
}
void ZoneBoss::die(const void* cause) {
    if (dungeon_ && kind_ == ZoneBossKind::Yog) {
        for (auto& mob : dungeon_->level().mobsMutable()) {
            if (ZoneMob* zone = dynamic_cast<ZoneMob*>(mob.get())) {
                if (zone->isAlive() && (zone->kind() == ZoneMobKind::RottingFist || zone->kind() == ZoneMobKind::BurningFist)) {
                    zone->damage(zone->HP, cause);
                }
            }
        }
    }

    Mob::die(cause);
    if (!dungeon_) return;

    dungeon_->level().drop(std::unique_ptr<Item>(new SkeletonKey(dungeon_->depth())), pos);
    if (kind_ == ZoneBossKind::Tengu) {
        const Hero& hero = dungeon_->hero();
        const Badge mastery = static_cast<Badge>(static_cast<int>(Badge::MasteryWarrior) + static_cast<int>(hero.heroClass()));
        if (!Progression::instance().hasBadge(mastery) || hero.subClass() != HeroSubClass::None) {
            dungeon_->level().drop(std::unique_ptr<Item>(new SpecialItem(SpecialKind::TomeOfMastery)), pos);
        }
    }
    if (kind_ == ZoneBossKind::King) {
        dungeon_->level().drop(std::unique_ptr<Item>(new SpecialItem(SpecialKind::ArmorKit)), pos);
    }
    if (RegularLevel* level = dynamic_cast<RegularLevel*>(&dungeon_->level())) level->unsealBossEntrance();
}

bool ZoneBoss::immuneTo(const std::type_info& type) const {
    if (kind_ == ZoneBossKind::DM300 && type == typeid(ToxicGas)) return true;
    if (kind_ == ZoneBossKind::King && (type == typeid(Paralysis) || type == typeid(Vertigo))) return true;
    if (kind_ == ZoneBossKind::Yog &&
        (type == typeid(Terror) || type == typeid(Amok) || type == typeid(Charm) || type == typeid(Sleep) ||
         type == typeid(Burning) || type == typeid(ToxicGas))) return true;
    return Mob::immuneTo(type);
}
bool ZoneBoss::resistantTo(const std::type_info& type) const {
    return (kind_ == ZoneBossKind::Tengu && (type == typeid(Poison) || type == typeid(ToxicGas))) ||
        (kind_ == ZoneBossKind::King && type == typeid(ToxicGas));
}
const char*ZoneBoss::description()const{return "One of the dungeon's guardians.";}
}
