#pragma once
#include <array>
#include "game/levels/Level.h"
namespace pdv {
class Dungeon;
class Ballistica {
public:
    static int cast(const Dungeon& dungeon, int from, int to, bool magic, bool hitChars);
    static const std::array<int, Level::Width>& trace(){ return trace_; }
    static int distance(){ return distance_; }
private:
    static std::array<int, Level::Width> trace_;
    static int distance_;
};
} // namespace pdv
