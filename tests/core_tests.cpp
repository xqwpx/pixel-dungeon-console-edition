#include <cassert>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "core/GameSession.h"
#include "game/Dungeon.h"
#include "game/actors/Actor.h"
#include "game/actors/Char.h"
#include "game/actors/mobs/Bestiary.h"
#include "game/actors/mobs/Mob.h"
#include "game/actors/mobs/Rat.h"
#include "game/actors/mobs/Piranha.h"
#include "game/actors/mobs/SpecialMob.h"
#include "game/items/weapon/Weapon.h"
#include "game/levels/Level.h"
#include "game/levels/Room.h"
#include "game/levels/SewerLevel.h"
#include "game/levels/Terrain.h"
#include "game/mechanics/ShadowCaster.h"
#include "game/pathfinding/PathFinder.h"
#include "game/utils/Random.h"

using namespace pdv;

static void terrainTests() {
    const auto& flags = Terrain::flags();

    assert(
        (flags[Terrain::WALL] &
         Terrain::PASSABLE) == 0
    );

    assert(
        flags[Terrain::WALL] &
        Terrain::LOS_BLOCKING
    );

    assert(
        flags[Terrain::DOOR] &
        Terrain::PASSABLE
    );

    assert(
        flags[Terrain::WATER] &
        Terrain::LIQUID
    );

    assert(
        Terrain::discover(
            Terrain::SECRET_DOOR
        ) == Terrain::DOOR
    );
}

static void fixtureTests() {
    static_assert(
        Level::Width == 32,
        ""
    );

    static_assert(
        Level::Height == 32,
        ""
    );

    Level level;
    level.createCoreParityTestMap();

    assert(
        level.passable(
            level.entrance()
        )
    );

    assert(
        level.passable(
            level.exit()
        )
    );

    assert(
        Level::distance(
            10 + 10 * 32,
            13 + 12 * 32
        ) == 3
    );

    assert(
        Level::adjacent(
            10 + 10 * 32,
            11 + 11 * 32
        )
    );
}

static void roomTests() {
    Room a(0, 0, 8, 8);
    Room b(8, 0, 16, 8);

    a.addNeighbour(&b);

    assert(
        a.neighbours.size() == 1 &&
        b.neighbours.size() == 1
    );

    assert(
        a.connect(&b)
    );

    assert(
        a.connected[&b] ==
        b.connected[&a]
    );
}

static void pathTests() {
    PathFinder::setMapSize(
        Level::Width,
        Level::Height
    );

    std::array<bool, Level::Length> pass{};
    pass.fill(false);

    for (int y = 1; y < 31; ++y) {
        for (int x = 1; x < 31; ++x) {
            pass[
                static_cast<std::size_t>(
                    y * 32 + x
                )
            ] = true;
        }
    }

    const int from =
        10 + 10 * 32;

    const int to =
        13 + 13 * 32;

    const int step =
        PathFinder::getStep(
            from,
            to,
            pass
        );

    assert(
        step == 11 + 11 * 32
    );

    const auto path =
        PathFinder::find(
            from,
            to,
            pass
        );

    assert(
        path.size() == 3 &&
        path.back() == to
    );
}

static void shadowTests() {
    std::array<bool, Level::Length> los{};
    std::array<bool, Level::Length> fov{};

    los.fill(false);
    fov.fill(false);

    los[10 * 32 + 11] = true;

    ShadowCaster::castShadow(
        10,
        10,
        los,
        fov,
        8
    );

    assert(
        fov[10 * 32 + 10]
    );

    assert(
        fov[10 * 32 + 11]
    );

    assert(
        !fov[10 * 32 + 12]
    );
}

struct TestActor : Actor {
    std::string name;
    std::vector<std::string>* log;
    float cost;
    bool waiting;

    TestActor(
        std::string n,
        std::vector<std::string>* l,
        float c,
        bool w = false
    )
        : name(std::move(n)),
          log(l),
          cost(c),
          waiting(w) {}

    bool act() override {
        log->push_back(name);

        if (waiting) {
            return false;
        }

        spend(cost);
        next();

        return true;
    }
};

static void actorTests() {
    ActorScheduler scheduler;

    std::vector<std::string> log;

    TestActor a(
        "A",
        &log,
        100
    );

    TestActor b(
        "B",
        &log,
        100
    );

    TestActor hero(
        "Hero",
        &log,
        1,
        true
    );

    scheduler.addDelayed(&a, 0);
    scheduler.addDelayed(&b, 1);
    scheduler.addDelayed(&hero, 2);

    scheduler.process();

    assert(
        log.size() == 3
    );

    assert(
        log[0] == "A" &&
        log[1] == "B" &&
        log[2] == "Hero"
    );
}

static void ratDefinitionTests() {
    std::unique_ptr<Mob> base =
        Bestiary::mob(1);

    assert(base);

    Rat* rat =
        dynamic_cast<Rat*>(
            base.get()
        );

    assert(rat);

    assert(
        rat->name ==
        "marsupial rat"
    );

    assert(
        rat->HP == 8 &&
        rat->HT == 8
    );

    assert(
        rat->defenseSkillValue() == 3
    );

    assert(
        rat->attackSkill(*rat) == 8
    );

    assert(
        rat->dr() == 1
    );

    assert(
        rat->maxLevel() == 5
    );

    Random::seed(77);

    for (int i = 0; i < 100; ++i) {
        const int damage =
            rat->damageRoll();

        assert(
            damage >= 1 &&
            damage <= 5
        );
    }
}

static void sewerGenerationTests() {
    PathFinder::setMapSize(
        Level::Width,
        Level::Height
    );

    int totalWater = 0;
    int totalGrass = 0;

    for (uint32_t seed = 1;
         seed <= 80;
         ++seed) {

        SewerLevel level;

        assert(
            level.create(1, seed)
        );

        assert(
            level.feeling() ==
            Level::Feeling::None
        );

        assert(
            level.secretDoors() == 0
        );

        assert(
            level.inBoundsCell(
                level.entrance()
            )
        );

        assert(
            level.inBoundsCell(
                level.exit()
            )
        );

        assert(
            level.entrance() !=
            level.exit()
        );

        assert(
            level.passable(
                level.entrance()
            )
        );

        assert(
            level.passable(
                level.exit()
            )
        );

        assert(
            level.rooms().size() >= 8
        );

        int standardRooms = 0;
        int entrances = 0;
        int exits = 0;

        for (const auto& room :
             level.rooms()) {

            if (room->type ==
                Room::Type::Standard) {
                ++standardRooms;
            }

            assert(
                room->type !=
                Room::Type::Shop
            );

            // Special rooms are now part of regular sewer generation.
        }

        assert(
            standardRooms >= 4
        );

        for (int i = 0;
             i < Level::Length;
             ++i) {

            const int terrain =
                level.cell(i);

            if (terrain ==
                Terrain::ENTRANCE) {
                ++entrances;
            }

            if (terrain ==
                Terrain::EXIT) {
                ++exits;
            }

            if (terrain >=
                    Terrain::WATER_TILES &&
                terrain <=
                    Terrain::WATER) {
                ++totalWater;
            }

            if (terrain ==
                    Terrain::GRASS ||
                terrain ==
                    Terrain::HIGH_GRASS) {
                ++totalGrass;
            }
        }

        assert(
            entrances == 1 &&
            exits == 1
        );

        const auto pass =
            level.passableMap();

        const auto path =
            PathFinder::find(
                level.entrance(),
                level.exit(),
                pass
            );

        assert(
            !path.empty() &&
            path.back() ==
            level.exit()
        );

        // Original depth-1 RegularLevel:
        // 2 + 1 % 5 + Random.Int(3) => 3..5 mobs.
        std::set<int> occupied;
        int regularMobs = 0;

        for (const auto& mob :
             level.mobs()) {

            assert(mob);

            if (dynamic_cast<Rat*>(mob.get())) {
                ++regularMobs;
            } else if (const SpecialMob* special = dynamic_cast<const SpecialMob*>(mob.get())) {
                assert(special->kind() == SpecialMobKind::Statue);
            } else {
                assert(dynamic_cast<Piranha*>(mob.get()));
                assert(static_cast<const Level&>(level).water(mob->pos));
            }

            assert(
                level.passable(
                    mob->pos
                )
            );

            assert(
                mob->pos !=
                level.entrance()
            );

            assert(
                occupied.insert(
                    mob->pos
                ).second
            );
        }

        assert(regularMobs >= 3 && regularMobs <= 5);

        for (int x = 0;
             x < Level::Width;
             ++x) {

            assert(
                !level.passable(x)
            );

            assert(
                !level.passable(
                    (Level::Height - 1) *
                    Level::Width +
                    x
                )
            );
        }
    }

    assert(
        totalWater > 0 &&
        totalGrass > 0
    );
}

static void dungeonOccupancyTests() {
    Dungeon dungeon;

    assert(
        dungeon.startNew(
            HeroClass::Warrior,
            0x12345678u
        )
    );

    assert(
        dungeon.scheduler().findChar(
            dungeon.hero().pos
        ) ==
        &dungeon.hero()
    );

    assert(
        dungeon.level().mobs().size() >= 3 &&
        dungeon.level().mobs().size() <= 5
    );

    for (const auto& mob :
         dungeon.level().mobs()) {

        assert(mob);

        assert(
            dungeon.scheduler().findChar(
                mob->pos
            ) ==
            mob.get()
        );

        assert(
            mob->state() ==
            Mob::State::Sleeping
        );
    }
}

int main() {
    terrainTests();
    fixtureTests();
    roomTests();
    pathTests();
    shadowTests();
    actorTests();

    ratDefinitionTests();
    sewerGenerationTests();
    dungeonOccupancyTests();

    std::cout
        << "PIXEL DUNGEON CONSOLE EDITION v0.7 mob AI parity tests: OK\n";

    return 0;
}
