#include "game/levels/Terrain.h"

namespace pdv { namespace Terrain {

const std::array<int,256>& flags() {
    static const std::array<int,256> value = [] {
        std::array<int,256> f{};
        f.fill(0);

        f[CHASM] = AVOID | PIT | UNSTITCHABLE;
        f[EMPTY] = PASSABLE;
        f[GRASS] = PASSABLE | FLAMABLE;
        f[EMPTY_WELL] = PASSABLE;
        f[WATER] = PASSABLE | LIQUID | UNSTITCHABLE;
        f[WALL] = LOS_BLOCKING | SOLID | UNSTITCHABLE;
        f[DOOR] = PASSABLE | LOS_BLOCKING | FLAMABLE | SOLID | UNSTITCHABLE;
        f[OPEN_DOOR] = PASSABLE | FLAMABLE | UNSTITCHABLE;
        f[ENTRANCE] = PASSABLE;
        f[EXIT] = PASSABLE;
        f[EMBERS] = PASSABLE;
        f[LOCKED_DOOR] = LOS_BLOCKING | SOLID | UNSTITCHABLE;
        f[PEDESTAL] = PASSABLE | UNSTITCHABLE;
        f[WALL_DECO] = f[WALL];
        f[BARRICADE] = FLAMABLE | SOLID | LOS_BLOCKING;
        f[EMPTY_SP] = f[EMPTY] | UNSTITCHABLE;
        f[HIGH_GRASS] = PASSABLE | LOS_BLOCKING | FLAMABLE;
        f[EMPTY_DECO] = f[EMPTY];
        f[LOCKED_EXIT] = SOLID;
        f[UNLOCKED_EXIT] = PASSABLE;
        f[SIGN] = PASSABLE | FLAMABLE;
        f[WELL] = AVOID;
        f[STATUE] = SOLID;
        f[STATUE_SP] = f[STATUE] | UNSTITCHABLE;
        f[BOOKSHELF] = f[BARRICADE] | UNSTITCHABLE;
        f[ALCHEMY] = PASSABLE;

        f[CHASM_WALL] = f[CHASM];
        f[CHASM_FLOOR] = f[CHASM];
        f[CHASM_FLOOR_SP] = f[CHASM];
        f[CHASM_WATER] = f[CHASM];

        f[SECRET_DOOR] = f[WALL] | SECRET | UNSTITCHABLE;
        f[TOXIC_TRAP] = AVOID;
        f[SECRET_TOXIC_TRAP] = f[EMPTY] | SECRET;
        f[FIRE_TRAP] = AVOID;
        f[SECRET_FIRE_TRAP] = f[EMPTY] | SECRET;
        f[PARALYTIC_TRAP] = AVOID;
        f[SECRET_PARALYTIC_TRAP] = f[EMPTY] | SECRET;
        f[POISON_TRAP] = AVOID;
        f[SECRET_POISON_TRAP] = f[EMPTY] | SECRET;
        f[ALARM_TRAP] = AVOID;
        f[SECRET_ALARM_TRAP] = f[EMPTY] | SECRET;
        f[LIGHTNING_TRAP] = AVOID;
        f[SECRET_LIGHTNING_TRAP] = f[EMPTY] | SECRET;
        f[GRIPPING_TRAP] = AVOID;
        f[SECRET_GRIPPING_TRAP] = f[EMPTY] | SECRET;
        f[SUMMONING_TRAP] = AVOID;
        f[SECRET_SUMMONING_TRAP] = f[EMPTY] | SECRET;
        f[INACTIVE_TRAP] = f[EMPTY];

        for (int i = WATER_TILES; i < WATER_TILES + 16; ++i) f[i] = f[WATER];
        return f;
    }();
    return value;
}

int discover(int terr) {
    switch (terr) {
        case SECRET_DOOR: return DOOR;
        case SECRET_FIRE_TRAP: return FIRE_TRAP;
        case SECRET_PARALYTIC_TRAP: return PARALYTIC_TRAP;
        case SECRET_TOXIC_TRAP: return TOXIC_TRAP;
        case SECRET_POISON_TRAP: return POISON_TRAP;
        case SECRET_ALARM_TRAP: return ALARM_TRAP;
        case SECRET_LIGHTNING_TRAP: return LIGHTNING_TRAP;
        case SECRET_GRIPPING_TRAP: return GRIPPING_TRAP;
        case SECRET_SUMMONING_TRAP: return SUMMONING_TRAP;
        default: return terr;
    }
}

} } // namespace pdv::Terrain
