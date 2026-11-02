#include <cassert>
#include <cmath>
#include <iostream>

#include "core/GameSession.h"
#include "game/Dungeon.h"
#include "game/actors/Hero.h"
#include "game/actors/buffs/Amok.h"
#include "game/actors/buffs/Buff.h"
#include "game/actors/buffs/Burning.h"
#include "game/actors/buffs/Frost.h"
#include "game/actors/buffs/Light.h"
#include "game/actors/buffs/MindVision.h"
#include "game/actors/buffs/Paralysis.h"
#include "game/actors/buffs/Shadows.h"
#include "game/actors/buffs/Sleep.h"
#include "game/actors/buffs/Slow.h"
#include "game/actors/buffs/Speed.h"
#include "game/actors/buffs/Vertigo.h"
#include "game/actors/mobs/Mob.h"
#include "game/levels/Level.h"
#include "game/ui/BuffIndicator.h"
#include "game/utils/Random.h"

using namespace pdv;

class DarkLevel : public Level {
public:
    int viewDistance() const override {
        return 3;
    }
};

static void slowSpeedTimeScaling() {
    Hero hero;
    hero.initialize(
        HeroClass::Warrior,
        10 + 10 * Level::Width
    );

    hero.resetTime();

    Slow* slow =
        Buff::prolong<Slow>(
            hero,
            Slow::Duration
        );

    assert(slow);

    hero.spend(10.0f);

    // Original Char.spend:
    // Slow => timeScale 0.5 => spend(10 / 0.5) = 20.
    assert(
        std::fabs(
            hero.time() - 20.0f
        ) < 0.0001f
    );

    slow->detach();
    hero.resetTime();

    Speed* speed =
        Buff::prolong<Speed>(
            hero,
            Speed::Duration
        );

    assert(speed);

    hero.spend(10.0f);

    // Speed => timeScale 2 => spend(10 / 2) = 5.
    assert(
        std::fabs(
            hero.time() - 5.0f
        ) < 0.0001f
    );

    speed->detach();
    hero.resetTime();

    slow =
        Buff::prolong<Slow>(
            hero,
            Slow::Duration
        );

    speed =
        Buff::prolong<Speed>(
            hero,
            Speed::Duration
        );

    hero.spend(10.0f);

    // Slow and Speed cancel each other's time scaling.
    assert(
        std::fabs(
            hero.time() - 10.0f
        ) < 0.0001f
    );

    slow->detach();
    speed->detach();
}

static void vertigoMovement() {
    Level level;
    level.createCoreParityTestMap();

    int start = -1;

    for (int y = 2;
         y < Level::Height - 2 &&
         start < 0;
         ++y) {

        for (int x = 2;
             x < Level::Width - 2;
             ++x) {

            const int candidate =
                y * Level::Width + x;

            if (!level.passable(candidate)) {
                continue;
            }

            bool allPassable = true;

            for (int offset :
                 Level::Neighbours8) {

                if (!level.passable(
                        candidate + offset
                    )) {
                    allPassable = false;
                    break;
                }
            }

            if (allPassable) {
                start = candidate;
                break;
            }
        }
    }

    assert(start >= 0);

    Hero hero;
    hero.initialize(
        HeroClass::Mage,
        start
    );

    Vertigo* vertigo =
        Buff::prolong<Vertigo>(
            hero,
            Vertigo::Duration
        );

    assert(vertigo);

    constexpr uint32_t Seed =
        0x130013u;

    Random::seed(Seed);

    const int expected =
        start +
        Level::Neighbours8[
            static_cast<std::size_t>(
                Random::Int(8)
            )
        ];

    Random::seed(Seed);

    assert(
        hero.moveTo(
            start + 1,
            level
        )
    );

    assert(
        hero.pos == expected
    );

    vertigo->detach();
}

static void frostParity() {
    Hero hero;
    hero.initialize(
        HeroClass::Warrior,
        100
    );

    Burning* burning =
        Buff::append<Burning>(
            hero
        );

    assert(burning);
    assert(hero.hasBuff(typeid(Burning)));

    Frost* frost =
        Buff::prolong<Frost>(
            hero,
            Frost::Duration
        );

    assert(frost);

    // Frost immediately extinguishes Burning.
    assert(
        !hero.hasBuff(typeid(Burning))
    );

    assert(hero.paralysed);

    Paralysis* paralysis =
        Buff::prolong<Paralysis>(
            hero,
            Paralysis::Duration
        );

    assert(paralysis);
    assert(hero.paralysed);

    // Any damage event thaws Frost first.
    hero.damage(0, nullptr);

    assert(
        !hero.hasBuff(typeid(Frost))
    );

    // Parallel Paralysis still owns the flag.
    assert(hero.paralysed);

    paralysis->detach();

    assert(!hero.paralysed);
}

static void lightParity() {
    DarkLevel level;

    Hero hero;
    hero.initialize(
        HeroClass::Mage,
        100
    );

    hero.bindLevel(&level);
    hero.viewDistance =
        level.viewDistance();

    assert(hero.viewDistance == 3);

    Light* light =
        Buff::prolong<Light>(
            hero,
            Light::Duration
        );

    assert(light);

    assert(
        hero.viewDistance ==
        Light::Distance
    );

    light->detach();

    assert(
        hero.viewDistance == 3
    );
}

static void shadowsParity() {
    Hero hero;
    hero.initialize(
        HeroClass::Rogue,
        100
    );

    Shadows* shadows =
        Buff::append<Shadows>(
            hero
        );

    assert(shadows);

    shadows->prolong();

    assert(hero.invisible == 1);
    assert(shadows->left() == 2.0f);

    shadows->act();

    assert(shadows->attached());
    assert(shadows->left() == 1.0f);

    shadows->act();

    assert(!shadows->attached());
    assert(hero.invisible == 0);
}

static void mindVisionAndShadowsFov() {
    Dungeon dungeon;

    assert(
        dungeon.startNew(
            HeroClass::Mage,
            0x1300ABCDu
        )
    );

    assert(
        !dungeon.level()
            .mobs()
            .empty()
    );

    Mob* farMob =
        dungeon.level()
            .mobsMutable()
            .front()
            .get();

    assert(farMob);

    MindVision* mind =
        Buff::prolong<MindVision>(
            dungeon.hero(),
            MindVision::Duration
        );

    assert(mind);

    dungeon.observe();

    assert(
        dungeon.visible()[
            static_cast<std::size_t>(
                farMob->pos
            )
        ]
    );

    mind->detach();

    Shadows* shadows =
        Buff::append<Shadows>(
            dungeon.hero()
        );

    assert(shadows);
    shadows->prolong();

    dungeon.observe();

    int visibleCount = 0;

    for (bool value :
         dungeon.visible()) {
        if (value) {
            ++visibleCount;
        }
    }

    // Shadows suppresses normal sight, leaving only sense=1 (3x3 maximum,
    // potentially clipped by discoverability).
    assert(visibleCount <= 9);

    shadows->detach();
}

static void amokAndSleepAi() {
    Dungeon dungeon;

    assert(
        dungeon.startNew(
            HeroClass::Warrior,
            0x13001111u
        )
    );

    auto& mobs =
        dungeon.level()
            .mobsMutable();

    assert(mobs.size() >= 2);

    Mob* first =
        mobs[0].get();

    Mob* second =
        mobs[1].get();

    assert(first && second);

    // Put both mobs in visible neighbouring cells around the hero.
    const int center =
        dungeon.hero().pos;

    int firstCell = -1;
    int secondCell = -1;

    for (int offset :
         Level::Neighbours8) {

        const int cell =
            center + offset;

        if (!dungeon.level().inBoundsCell(cell) ||
            !dungeon.level().passable(cell)) {
            continue;
        }

        if (firstCell < 0) {
            firstCell = cell;
        } else {
            secondCell = cell;
            break;
        }
    }

    assert(firstCell >= 0);
    assert(secondCell >= 0);

    first->pos = firstCell;
    second->pos = secondCell;

    Amok* amok =
        Buff::prolong<Amok>(
            *first,
            5.0f
        );

    assert(amok);

    assert(
        first->state() ==
        Mob::State::Hunting
    );

    // act() recomputes its FOV and chooseEnemy() must prefer another visible
    // mob instead of Dungeon.hero while Amok is active.
    first->act();

    assert(first->enemy() != nullptr);
    assert(first->enemy() != &dungeon.hero());

    amok->detach();

    Sleep* sleep =
        Buff::prolong<Sleep>(
            *first,
            5.0f
        );

    assert(sleep);

    assert(
        first->state() ==
        Mob::State::Sleeping
    );

    assert(
        first->cooldown() >=
        Sleep::SWS
    );

    sleep->detach();
}

static void buffIndicatorParity() {
    Hero hero;
    hero.initialize(
        HeroClass::Warrior,
        100
    );

    Buff::prolong<Frost>(
        hero,
        Frost::Duration
    );

    Buff::prolong<Vertigo>(
        hero,
        Vertigo::Duration
    );

    Buff::prolong<Light>(
        hero,
        Light::Duration
    );

    const auto icons =
        BuffIndicator::iconsFor(hero);

    bool hasFrost = false;
    bool hasVertigo = false;
    bool hasLight = false;

    for (int icon : icons) {
        hasFrost =
            hasFrost ||
            icon == BuffIndicator::Frost;

        hasVertigo =
            hasVertigo ||
            icon == BuffIndicator::Vertigo;

        hasLight =
            hasLight ||
            icon == BuffIndicator::Light;
    }

    assert(hasFrost);
    assert(hasVertigo);
    assert(hasLight);

    static_assert(
        BuffIndicator::Size == 7,
        "Original BuffIndicator icon size is 7"
    );

    static_assert(
        BuffIndicator::MindVision == 0 &&
        BuffIndicator::Frost == 15 &&
        BuffIndicator::Light == 22 &&
        BuffIndicator::Vertigo == 29,
        "Buff icon indices must match upstream atlas"
    );
}

int main() {
    slowSpeedTimeScaling();
    vertigoMovement();
    frostParity();
    lightParity();
    shadowsParity();
    mindVisionAndShadowsFov();
    amokAndSleepAi();
    buffIndicatorParity();

    std::cout
        << "PIXEL DUNGEON CONSOLE EDITION v0.13 extended buff + vision tests: OK\n";

    return 0;
}
