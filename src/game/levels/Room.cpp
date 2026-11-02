#include "game/levels/Room.h"
#include "game/utils/Random.h"
#include <algorithm>
namespace pdv {
Point Room::random(int margin) const {
    return Point(Random::Int(left + 1 + margin, right - margin),
                 Random::Int(top + 1 + margin, bottom - margin));
}
void Room::addNeighbour(Room* other){
    if(!other || other==this) return;
    Rect inter=intersect(*other);
    // Original Room.addNeighbour: rectangles must share a segment at least 3 cells long.
    if((inter.width()==0 && inter.height()>=3) || (inter.height()==0 && inter.width()>=3)){
        if(std::find(neighbours.begin(),neighbours.end(),other)==neighbours.end()) neighbours.push_back(other);
        if(std::find(other->neighbours.begin(),other->neighbours.end(),this)==other->neighbours.end()) other->neighbours.push_back(this);
    }
}
bool Room::connect(Room* other){
    if(!other || other==this) return false;
    if(connected.find(other)!=connected.end()) return false;
    connected[other]=nullptr;
    other->connected[this]=nullptr;
    return true;
}
bool Room::insideCell(const Point& p) const {
    return p.x>left && p.x<right && p.y>top && p.y<bottom;
}
Point Room::center() const {
    return Point(
        (left + right) / 2 + (((right - left) & 1) == 1 ? Random::Int(2) : 0),
        (top + bottom) / 2 + (((bottom - top) & 1) == 1 ? Random::Int(2) : 0)
    );
}
std::vector<std::shared_ptr<Room::Door>> Room::doors() const {
    std::vector<std::shared_ptr<Door>> r;
    for(const auto& kv:connected) r.push_back(kv.second);
    return r;
}
}
