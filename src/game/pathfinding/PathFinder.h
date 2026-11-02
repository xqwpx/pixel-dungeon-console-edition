#pragma once
#include <array>
#include <vector>
#include "game/levels/LevelConstants.h"

namespace pdv {

class PathFinder {
public:
    static void setMapSize(int width, int height);
    static std::vector<int> find(int from, int to, const std::array<bool,LevelLength>& passable);
    static int getStep(int from, int to, const std::array<bool,LevelLength>& passable);
    static void buildDistanceMap(int to, const std::array<bool,LevelLength>& passable, int limit);
    static const std::vector<int>& distance();

private:
    static bool buildDistanceMapInternal(int from, int to, const std::array<bool,LevelLength>& passable);
};

} // namespace pdv
