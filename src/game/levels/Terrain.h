#pragma once
#include <array>

namespace pdv { namespace Terrain {

constexpr int CHASM = 0;
constexpr int EMPTY = 1;
constexpr int GRASS = 2;
constexpr int EMPTY_WELL = 3;
constexpr int WALL = 4;
constexpr int DOOR = 5;
constexpr int OPEN_DOOR = 6;
constexpr int ENTRANCE = 7;
constexpr int EXIT = 8;
constexpr int EMBERS = 9;
constexpr int LOCKED_DOOR = 10;
constexpr int PEDESTAL = 11;
constexpr int WALL_DECO = 12;
constexpr int BARRICADE = 13;
constexpr int EMPTY_SP = 14;
constexpr int HIGH_GRASS = 15;
constexpr int SECRET_DOOR = 16;
constexpr int TOXIC_TRAP = 17;
constexpr int SECRET_TOXIC_TRAP = 18;
constexpr int FIRE_TRAP = 19;
constexpr int SECRET_FIRE_TRAP = 20;
constexpr int PARALYTIC_TRAP = 21;
constexpr int SECRET_PARALYTIC_TRAP = 22;
constexpr int INACTIVE_TRAP = 23;
constexpr int EMPTY_DECO = 24;
constexpr int LOCKED_EXIT = 25;
constexpr int UNLOCKED_EXIT = 26;
constexpr int POISON_TRAP = 27;
constexpr int SECRET_POISON_TRAP = 28;
constexpr int SIGN = 29;
constexpr int ALARM_TRAP = 30;
constexpr int SECRET_ALARM_TRAP = 31;
constexpr int LIGHTNING_TRAP = 32;
constexpr int SECRET_LIGHTNING_TRAP = 33;
constexpr int WELL = 34;
constexpr int STATUE = 35;
constexpr int STATUE_SP = 36;
constexpr int GRIPPING_TRAP = 37;
constexpr int SECRET_GRIPPING_TRAP = 38;
constexpr int SUMMONING_TRAP = 39;
constexpr int SECRET_SUMMONING_TRAP = 40;
constexpr int BOOKSHELF = 41;
constexpr int ALCHEMY = 42;
constexpr int CHASM_FLOOR = 43;
constexpr int CHASM_FLOOR_SP = 44;
constexpr int CHASM_WALL = 45;
constexpr int CHASM_WATER = 46;
constexpr int WATER_TILES = 48;
constexpr int WATER = 63;

constexpr int PASSABLE = 0x01;
constexpr int LOS_BLOCKING = 0x02;
constexpr int FLAMABLE = 0x04;
constexpr int SECRET = 0x08;
constexpr int SOLID = 0x10;
constexpr int AVOID = 0x20;
constexpr int LIQUID = 0x40;
constexpr int PIT = 0x80;
constexpr int UNSTITCHABLE = 0x100;

const std::array<int,256>& flags();
int discover(int terrain);
inline int flagsFor(int terrain) {
    return terrain >= 0 && terrain < 256 ? flags()[static_cast<std::size_t>(terrain)] : 0;
}

} } // namespace pdv::Terrain
