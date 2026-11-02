#include <cassert>
#include <cmath>
#include <iostream>

#include "core/GameSession.h"
#include "game/Dungeon.h"
#include "game/actors/Actor.h"
#include "game/actors/Hero.h"
#include "game/actors/buffs/Bleeding.h"
#include "game/actors/buffs/Blindness.h"
#include "game/actors/buffs/Buff.h"
#include "game/actors/buffs/Burning.h"
#include "game/actors/buffs/Charm.h"
#include "game/actors/buffs/Cripple.h"
#include "game/actors/buffs/Hunger.h"
#include "game/actors/buffs/Invisibility.h"
#include "game/actors/buffs/Paralysis.h"
#include "game/actors/buffs/Poison.h"
#include "game/actors/buffs/Regeneration.h"
#include "game/actors/buffs/Roots.h"
#include "game/actors/buffs/Terror.h"
#include "game/actors/buffs/Weakness.h"
#include "game/actors/mobs/Mob.h"
#include "game/actors/mobs/Rat.h"
#include "game/levels/Level.h"
#include "game/utils/Random.h"

using namespace pdv;

static void actorIdTests() {
    ActorScheduler scheduler;

    Hero hero;
    hero.initialize(HeroClass::Warrior, 100);

    Rat rat;
    rat.pos = 101;

    scheduler.add(&hero);
    scheduler.add(&rat);

    const int heroId = hero.id();
    const int ratId = rat.id();

    assert(heroId > 0);
    assert(ratId > 0);
    assert(heroId != ratId);

    assert(scheduler.findById(heroId) == &hero);
    assert(scheduler.findById(ratId) == &rat);

    scheduler.clear();
}

static void flavourAndFlagsTests() {
    Hero hero;
    hero.initialize(HeroClass::Warrior, 100);

    assert(hero.invisible == 0);

    Invisibility* invis =
        Buff::prolong<Invisibility>(
            hero,
            Invisibility::Duration
        );

    assert(invis);
    assert(hero.invisible == 1);
    assert(hero.hasBuff(typeid(Invisibility)));

    invis->detach();

    assert(hero.invisible == 0);
    assert(!hero.hasBuff(typeid(Invisibility)));

    Paralysis* paralysis =
        Buff::prolong<Paralysis>(
            hero,
            Paralysis::Duration
        );

    assert(paralysis);
    assert(hero.paralysed);

    paralysis->detach();
    assert(!hero.paralysed);

    Roots* roots =
        Buff::prolong<Roots>(
            hero,
            5.0f
        );

    assert(roots);
    assert(hero.rooted);

    roots->detach();
    assert(!hero.rooted);

    hero.flying = true;

    assert(
        Buff::prolong<Roots>(
            hero,
            5.0f
        ) == nullptr
    );

    hero.flying = false;
}

static void weaknessAndCrippleTests() {
    Hero hero;
    hero.initialize(HeroClass::Warrior, 100);

    assert(hero.STR() == 11);

    Weakness* weak =
        Buff::prolong<Weakness>(
            hero,
            Weakness::Duration
        );

    assert(weak);
    assert(hero.STR() == 9);

    weak->detach();
    assert(hero.STR() == 11);

    const float base = hero.speed();

    Cripple* cripple =
        Buff::prolong<Cripple>(
            hero,
            Cripple::Duration
        );

    assert(cripple);

    assert(
        std::fabs(
            hero.speed() -
            base * 0.5f
        ) < 0.0001f
    );

    cripple->detach();

    assert(
        std::fabs(
            hero.speed() -
            base
        ) < 0.0001f
    );
}

static void poisonBleedingBurningTests() {
    Random::seed(0x121212u);

    Level level;
    level.createCoreParityTestMap();

    Hero poisonHero;
    poisonHero.initialize(HeroClass::Mage, 100);
    poisonHero.bindLevel(&level);

    Poison* poison =
        Buff::append<Poison>(
            poisonHero
        );

    assert(poison);

    poison->set(6.0f);

    const int poisonBefore =
        poisonHero.HP;

    poison->act();

    assert(
        poisonHero.HP ==
        poisonBefore - 3
    );

    assert(
        std::fabs(
            poison->left() - 5.0f
        ) < 0.0001f
    );

    poison->detach();

    Hero bleedingHero;
    bleedingHero.initialize(HeroClass::Mage, 100);

    Bleeding* bleeding =
        Buff::append<Bleeding>(
            bleedingHero
        );

    assert(bleeding);

    bleeding->set(8);

    const int bleedingBefore =
        bleedingHero.HP;

    bleeding->act();

    assert(
        bleedingHero.HP <
        bleedingBefore
    );

    bleeding->detach();

    Hero burningHero;
    burningHero.initialize(HeroClass::Mage, 100);
    burningHero.bindLevel(&level);

    Burning* burning =
        Buff::append<Burning>(
            burningHero
        );

    assert(burning);

    const int burningBefore =
        burningHero.HP;

    burning->act();

    const int burningDamage =
        burningBefore -
        burningHero.HP;

    assert(burningDamage >= 1);
    assert(burningDamage <= 4);

    if (burning->attached()) {
        burning->detach();
    }
}

static void regenerationTests() {
    Hero hero;
    hero.initialize(HeroClass::Mage, 100);

    Hunger hunger;
    hunger.reset();
    hunger.bind(&hero);
    hero.bindHunger(&hunger);

    hero.HP = 10;

    Regeneration* regeneration =
        Buff::affect<Regeneration>(
            hero
        );

    assert(regeneration);

    regeneration->act();

    assert(hero.HP == 11);

    regeneration->detach();
    hunger.detach();
}

static void blindnessTests() {
    Dungeon dungeon;

    assert(
        dungeon.startNew(
            HeroClass::Warrior,
            0x120012u
        )
    );

    dungeon.observe();

    int normalVisible = 0;

    for (bool cell : dungeon.visible()) {
        if (cell) {
            ++normalVisible;
        }
    }

    Blindness* blindness =
        Buff::prolong<Blindness>(
            dungeon.hero(),
            5.0f
        );

    assert(blindness);

    dungeon.observe();

    int blindVisible = 0;

    for (bool cell : dungeon.visible()) {
        if (cell) {
            ++blindVisible;
        }
    }

    assert(blindVisible > 0);
    assert(blindVisible <= 9);
    assert(normalVisible > blindVisible);

    blindness->detach();
    dungeon.observe();

    int restoredVisible = 0;

    for (bool cell : dungeon.visible()) {
        if (cell) {
            ++restoredVisible;
        }
    }

    assert(restoredVisible > blindVisible);
}

static void terrorAndCharmTests() {
    Dungeon dungeon;

    assert(
        dungeon.startNew(
            HeroClass::Warrior,
            0x991212u
        )
    );

    assert(
        !dungeon.level()
            .mobsMutable()
            .empty()
    );

    Mob* mob =
        dungeon.level()
            .mobsMutable()
            .front()
            .get();

    assert(mob);

    const int heroId =
        dungeon.hero().id();

    Terror* terror =
        Buff::append<Terror>(
            *mob
        );

    assert(terror);

    terror->object = heroId;

    assert(
        mob->state() ==
        Mob::State::Fleeing
    );

    // Original Mob.damage() calls Terror.recover().
    // A newly appended Terror has cooldown < DURATION and is removed.
    mob->damage(
        0,
        &dungeon.hero()
    );

    assert(
        !mob->hasBuff(typeid(Terror))
    );

    assert(
        mob->state() ==
        Mob::State::Hunting
    );

    Charm* charm =
        Buff::prolong<Charm>(
            *mob,
            5.0f
        );

    assert(charm);

    charm->object = heroId;

    assert(
        mob->isCharmedBy(
            dungeon.hero()
        )
    );

    charm->detach();

    assert(
        !mob->isCharmedBy(
            dungeon.hero()
        )
    );
}

static void dungeonPersistentBuffTests() {
    Dungeon dungeon;

    assert(
        dungeon.startNew(
            HeroClass::Rogue,
            0x121299u
        )
    );

    assert(
        dungeon.hero()
            .hasBuff(
                typeid(Hunger)
            )
    );

    assert(
        dungeon.hero()
            .hasBuff(
                typeid(Regeneration)
            )
    );

    const int oldPos =
        dungeon.hero().pos;

    Paralysis* paralysis =
        Buff::prolong<Paralysis>(
            dungeon.hero(),
            2.0f
        );

    assert(paralysis);

    // A movement command while paralysed spends a turn but does not move.
    dungeon.moveHeroBy(1, 0);

    assert(
        dungeon.hero().pos ==
        oldPos
    );
}

int main() {
    actorIdTests();
    flavourAndFlagsTests();
    weaknessAndCrippleTests();
    poisonBleedingBurningTests();
    regenerationTests();
    blindnessTests();
    terrorAndCharmTests();
    dungeonPersistentBuffTests();

    std::cout
        << "PIXEL DUNGEON CONSOLE EDITION v0.12 buff + status parity tests: OK\n";

    return 0;
}
