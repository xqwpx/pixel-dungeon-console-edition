#include <cassert>
#include <iostream>
#include <memory>

#include "core/GameSession.h"
#include "game/Dungeon.h"
#include "game/actors/Hero.h"
#include "game/actors/mobs/Mob.h"
#include "game/actors/blobs/Blob.h"
#include "game/actors/blobs/Fire.h"
#include "game/actors/blobs/ToxicGas.h"
#include "game/actors/blobs/ParalyticGas.h"
#include "game/actors/buffs/Awareness.h"
#include "game/actors/buffs/Bleeding.h"
#include "game/actors/buffs/Buff.h"
#include "game/actors/buffs/Burning.h"
#include "game/actors/buffs/Cripple.h"
#include "game/actors/buffs/GasesImmunity.h"
#include "game/actors/buffs/Invisibility.h"
#include "game/actors/buffs/Levitation.h"
#include "game/actors/buffs/MindVision.h"
#include "game/actors/buffs/Ooze.h"
#include "game/actors/buffs/Paralysis.h"
#include "game/actors/buffs/Poison.h"
#include "game/actors/buffs/Roots.h"
#include "game/actors/buffs/Vertigo.h"
#include "game/actors/buffs/Weakness.h"
#include "game/items/potions/PotionOfHealing.h"
#include "game/items/potions/PotionOfInvisibility.h"
#include "game/items/potions/PotionOfLevitation.h"
#include "game/items/potions/PotionOfLiquidFlame.h"
#include "game/items/potions/PotionOfMindVision.h"
#include "game/items/potions/PotionOfParalyticGas.h"
#include "game/items/potions/PotionOfPurity.h"
#include "game/items/potions/PotionOfToxicGas.h"
#include "game/levels/Terrain.h"
#include "game/levels/traps/AlarmTrap.h"
#include "game/levels/traps/GrippingTrap.h"
#include "game/levels/traps/PoisonTrap.h"
#include "game/pathfinding/PathFinder.h"
#include "game/utils/Random.h"

using namespace pdv;

static int adjacentPassable(
    const Level& level,
    int center
) {
    for (int off : Level::Neighbours8) {
        const int cell = center + off;

        if (level.inBoundsCell(cell) &&
            level.passable(cell)) {
            return cell;
        }
    }

    return -1;
}

static void levitationLandingTrap() {
    Dungeon dungeon;

    assert(
        dungeon.startNew(
            HeroClass::Mage,
            0x140001u
        )
    );

    Hero& hero = dungeon.hero();
    Level& level = dungeon.level();

    Roots* roots =
        Buff::prolong<Roots>(
            hero,
            5.0f
        );

    assert(roots);
    assert(hero.rooted);

    Levitation* lev =
        Buff::prolong<Levitation>(
            hero,
            Levitation::Duration
        );

    assert(lev);
    assert(hero.flying);
    assert(!hero.rooted);

    level.setRaw(
        hero.pos,
        Terrain::POISON_TRAP
    );
    level.buildFlagMaps();

    // While flying the plate is ignored by Hero movement. On landing,
    // Levitation.detach() presses the current cell.
    lev->detach();

    assert(!hero.flying);
    assert(
        level.cell(hero.pos) ==
        Terrain::INACTIVE_TRAP
    );

    Poison* poison =
        dynamic_cast<Poison*>(
            hero.findBuff(
                typeid(Poison)
            )
        );

    assert(poison);
    assert(poison->left() == 4.0f);
}

static void gasesImmunityParity() {
    Hero hero;
    hero.initialize(
        HeroClass::Warrior,
        100
    );

    GasesImmunity* immunity =
        Buff::prolong<GasesImmunity>(
            hero,
            GasesImmunity::Duration
        );

    assert(immunity);

    assert(
        Buff::prolong<Paralysis>(
            hero,
            5.0f
        ) == nullptr
    );

    assert(
        Buff::prolong<Vertigo>(
            hero,
            5.0f
        ) == nullptr
    );

    immunity->detach();

    assert(
        Buff::prolong<Paralysis>(
            hero,
            5.0f
        ) != nullptr
    );
}

static void oozeWaterParity() {
    Level level;
    level.createCoreParityTestMap();

    Hero hero;
    hero.initialize(
        HeroClass::Mage,
        24 + 8 * Level::Width
    );
    hero.bindLevel(&level);

    assert(level.water(hero.pos));

    Ooze* ooze =
        Buff::append<Ooze>(hero);

    assert(ooze);

    const int before = hero.HP;
    ooze->act();

    assert(hero.HP == before - 1);
    assert(!ooze->attached());
}

static void trapParity() {
    Dungeon dungeon;

    assert(
        dungeon.startNew(
            HeroClass::Warrior,
            0x140002u
        )
    );

    Hero& hero = dungeon.hero();
    Level& level = dungeon.level();

    // Gripping trap: depth 1, damage <= 4 and applies both effects.
    GrippingTrap::trigger(
        dungeon,
        hero.pos,
        &hero
    );

    assert(
        hero.hasBuff(
            typeid(Bleeding)
        )
    );

    assert(
        hero.hasBuff(
            typeid(Cripple)
        )
    );

    // Alarm trap beckons every other mob to its position.
    AlarmTrap::trigger(
        dungeon,
        hero.pos,
        &hero
    );

    for (const auto& mob :
         level.mobs()) {
        if (mob) {
            assert(
                mob->target() ==
                hero.pos
            );
        }
    }

    const int trapCell =
        adjacentPassable(
            level,
            hero.pos
        );

    assert(trapCell >= 0);

    level.setRaw(
        trapCell,
        Terrain::TOXIC_TRAP
    );
    level.buildFlagMaps();

    level.press(
        trapCell,
        &hero
    );

    assert(
        level.cell(trapCell) ==
        Terrain::INACTIVE_TRAP
    );

    Blob* toxic =
        level.blob(
            BlobKind::ToxicGas
        );

    assert(toxic);
    assert(
        toxic->valueAt(trapCell) ==
        320
    );
}

static void blobParity() {
    Dungeon dungeon;

    assert(
        dungeon.startNew(
            HeroClass::Mage,
            0x140003u
        )
    );

    Hero& hero = dungeon.hero();
    Level& level = dungeon.level();

    hero.HT = 100;
    hero.HP = 100;

    Blob* toxic =
        level.seedBlob(
            BlobKind::ToxicGas,
            hero.pos,
            1000
        );

    assert(toxic);

    Random::seed(123);
    const int before = hero.HP;

    toxic->act();

    assert(hero.HP < before);

    Buff::prolong<GasesImmunity>(
        hero,
        GasesImmunity::Duration
    );

    hero.HP = 100;
    level.seedBlob(
        BlobKind::ToxicGas,
        hero.pos,
        1000
    );

    toxic->act();

    assert(hero.HP == 100);

    Blob* par =
        level.seedBlob(
            BlobKind::ParalyticGas,
            hero.pos,
            1000
        );

    assert(par);

    par->act();

    // GasesImmunity blocks Paralysis attachment.
    assert(
        !hero.hasBuff(
            typeid(Paralysis)
        )
    );

    Buff::detach<GasesImmunity>(hero);

    level.seedBlob(
        BlobKind::ParalyticGas,
        hero.pos,
        1000
    );

    par->act();

    assert(
        hero.hasBuff(
            typeid(Paralysis)
        )
    );

    Buff::detach<Paralysis>(hero);

    Blob* fire =
        level.seedBlob(
            BlobKind::Fire,
            hero.pos,
            2
        );

    assert(fire);

    fire->act();

    assert(
        hero.hasBuff(
            typeid(Burning)
        )
    );
}

static void healingPotionParity() {
    Dungeon dungeon;

    assert(
        dungeon.startNew(
            HeroClass::Warrior,
            0x140004u
        )
    );

    Hero& hero = dungeon.hero();

    hero.HP = 5;

    Poison* poison =
        Buff::append<Poison>(hero);
    assert(poison);
    poison->set(5.0f);

    Buff::prolong<Cripple>(
        hero,
        5.0f
    );

    Buff::prolong<Weakness>(
        hero,
        5.0f
    );

    Bleeding* bleeding =
        Buff::append<Bleeding>(hero);
    assert(bleeding);
    bleeding->set(5);

    PotionOfHealing potion;
    potion.apply(
        dungeon,
        hero
    );

    assert(hero.HP == hero.HT);
    assert(!hero.hasBuff(typeid(Poison)));
    assert(!hero.hasBuff(typeid(Cripple)));
    assert(!hero.hasBuff(typeid(Weakness)));
    assert(!hero.hasBuff(typeid(Bleeding)));
    assert(potion.isKnown());
}

static void utilityPotionParity() {
    Dungeon dungeon;

    assert(
        dungeon.startNew(
            HeroClass::Mage,
            0x140005u
        )
    );

    Hero& hero = dungeon.hero();

    PotionOfMindVision mind;
    mind.apply(
        dungeon,
        hero
    );

    assert(
        hero.hasBuff(
            typeid(MindVision)
        )
    );

    PotionOfInvisibility invis;
    invis.apply(
        dungeon,
        hero
    );

    assert(
        hero.hasBuff(
            typeid(Invisibility)
        )
    );

    PotionOfLevitation lev;
    lev.apply(
        dungeon,
        hero
    );

    assert(
        hero.hasBuff(
            typeid(Levitation)
        )
    );

    assert(hero.flying);

    PotionOfPurity purity;
    purity.apply(
        dungeon,
        hero
    );

    assert(
        hero.hasBuff(
            typeid(GasesImmunity)
        )
    );
}

static void shatterPotionParity() {
    Dungeon dungeon;

    assert(
        dungeon.startNew(
            HeroClass::Rogue,
            0x140006u
        )
    );

    const int cell =
        dungeon.hero().pos;

    PotionOfToxicGas toxic;
    toxic.shatter(
        dungeon,
        cell
    );

    assert(
        dungeon.level()
            .blob(BlobKind::ToxicGas)
            ->valueAt(cell) ==
        1000
    );

    PotionOfParalyticGas par;
    par.shatter(
        dungeon,
        cell
    );

    assert(
        dungeon.level()
            .blob(BlobKind::ParalyticGas)
            ->valueAt(cell) ==
        1000
    );

    PotionOfLiquidFlame fire;
    fire.shatter(
        dungeon,
        cell
    );

    assert(
        dungeon.level()
            .blob(BlobKind::Fire)
            ->valueAt(cell) ==
        2
    );

    PotionOfPurity purity;
    purity.shatter(
        dungeon,
        cell
    );

    assert(
        dungeon.level()
            .blob(BlobKind::ToxicGas)
            ->valueAt(cell) ==
        0
    );

    assert(
        dungeon.level()
            .blob(BlobKind::ParalyticGas)
            ->valueAt(cell) ==
        0
    );
}

static void awarenessFovParity() {
    Dungeon dungeon;

    assert(
        dungeon.startNew(
            HeroClass::Warrior,
            0x140007u
        )
    );

    const int farCell =
        dungeon.level().exit();

    dungeon.level().drop(
        std::unique_ptr<Item>(
            new PotionOfHealing()
        ),
        farCell
    );

    Buff::prolong<Awareness>(
        dungeon.hero(),
        Awareness::Duration
    );

    dungeon.observe();

    assert(
        dungeon.visible()[
            static_cast<std::size_t>(
                farCell
            )
        ]
    );
}

int main() {
    PathFinder::setMapSize(
        Level::Width,
        Level::Height
    );

    levitationLandingTrap();
    gasesImmunityParity();
    oozeWaterParity();
    trapParity();
    blobParity();
    healingPotionParity();
    utilityPotionParity();
    shatterPotionParity();
    awarenessFovParity();

    std::cout
        << "PIXEL DUNGEON CONSOLE EDITION v0.14 traps + blobs + potions tests: OK\n";

    return 0;
}
