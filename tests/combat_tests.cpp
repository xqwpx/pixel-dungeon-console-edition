#include <cassert>
#include <iostream>
#include <memory>

#include "game/actors/Actor.h"
#include "game/actors/Char.h"
#include "game/actors/Hero.h"
#include "game/actors/mobs/Mob.h"
#include "game/actors/mobs/Rat.h"
#include "game/levels/Level.h"
#include "game/utils/Random.h"

using namespace pdv;

struct FixedChar : Char {
    int acu = 10;
    int def = 0;
    int armor = 0;
    int roll = 4;

    bool act() override { return false; }
    int attackSkill(const Char&) const override { return acu; }
    int defenseSkill(const Char&) const override { return def; }
    int dr() const override { return armor; }
    int damageRoll() const override { return roll; }
};

static void hitAndDamagePipeline() {
    Random::seed(1234);

    FixedChar attacker;
    FixedChar defender;

    attacker.name = "attacker";
    defender.name = "defender";

    attacker.acu = 10;
    attacker.roll = 4;

    defender.def = 0;
    defender.armor = 0;
    defender.HP = defender.HT = 10;

    assert(attacker.attack(defender));
    assert(defender.HP == 6);
}

static void deathRemovesActorAndCell() {
    Random::seed(9);

    ActorScheduler scheduler;

    FixedChar attacker;
    FixedChar defender;

    attacker.pos = 10 + 10 * Level::Width;
    defender.pos = 11 + 10 * Level::Width;

    attacker.roll = 20;
    attacker.acu = 10;

    defender.HP = defender.HT = 5;
    defender.def = 0;
    defender.armor = 0;

    scheduler.add(&attacker);
    scheduler.add(&defender);

    assert(scheduler.findChar(defender.pos) == &defender);

    assert(attacker.attack(defender));
    assert(!defender.isAlive());
    assert(scheduler.findChar(defender.pos) == nullptr);
}

static void heroBaseCombatValues() {
    Random::seed(22);

    Hero warrior;
    warrior.initialize(HeroClass::Warrior, 100);

    Rat rat;
    rat.pos = 101;

    assert(warrior.attackSkill(rat) == 10);
    assert(warrior.defenseSkill(rat) == 5);
    assert(warrior.dr() == 2);
    assert(warrior.STR() == 11);
    assert(warrior.attackDelay() == 1.0f);

    for (int i = 0; i < 100; ++i) {
        const int dmg = warrior.damageRoll();
        assert(dmg >= 1 && dmg <= 12);
    }

    Hero mage;
    mage.initialize(HeroClass::Mage, 100);
    assert(mage.dr() == 2);
    assert(mage.attackDelay() == 0.5f);

    for (int i = 0; i < 100; ++i) {
        const int dmg = mage.damageRoll();
        assert(dmg >= 1 && dmg <= 5);
    }
}

static void ratCombatValuesAndAwarenessDefense() {
    Rat rat;
    Hero hero;
    hero.initialize(HeroClass::Mage, 100);

    assert(rat.HP == 8 && rat.HT == 8);
    assert(rat.attackSkill(hero) == 8);
    assert(rat.dr() == 1);

    // Original Mob.defenseSkill returns zero before it has seen the enemy.
    assert(rat.defenseSkill(hero) == 0);

    rat.setState(Mob::State::Hunting);
    // enemySeen is driven by act(); the state alone does not grant defense.
    assert(rat.defenseSkill(hero) == 0);
}

static void mobDamageWakesSleepingMob() {
    Rat rat;
    assert(rat.state() == Mob::State::Sleeping);

    rat.damage(1, nullptr);

    assert(rat.HP == 7);
    assert(rat.state() == Mob::State::Wandering);
}

int main() {
    hitAndDamagePipeline();
    deathRemovesActorAndCell();
    heroBaseCombatValues();
    ratCombatValuesAndAwarenessDefense();
    mobDamageWakesSleepingMob();

    std::cout << "PIXEL DUNGEON CONSOLE EDITION v0.9 combat + equipment tests: OK\n";
    return 0;
}
