#pragma once
#include <vector>
namespace pdv {
class Room;
class Graph {
public:
    static void setPrice(const std::vector<Room*>& nodes,float value);
    static void buildDistanceMap(const std::vector<Room*>& nodes,Room* focus);
    static std::vector<Room*> buildPath(const std::vector<Room*>& nodes,Room* from,Room* to);
};
}
