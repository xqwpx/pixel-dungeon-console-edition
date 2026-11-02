#pragma once
namespace pdv {
class Dungeon; class Char;
class ParalyticTrap {
public:
    static void trigger(Dungeon& dungeon, int pos, Char* ch);
};
}
