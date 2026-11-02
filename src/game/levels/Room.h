#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include "game/utils/Rect.h"
namespace pdv {
class Room : public Rect {
public:
    enum class Type {
        Null, Standard, Entrance, Exit, BossExit, Tunnel, Passage,
        Shop, Blacksmith, Treasury, Armory, Library, Laboratory, Vault,
        Traps, Storage, MagicWell, Garden, Crypt, Statue, Pool,
        RatKing, WeakFloor, Pit, Altar
    };
    class Door {
    public:
        enum class Type { Empty=0, Tunnel=1, Regular=2, Unlocked=3, Hidden=4, Barricade=5, Locked=6 };
        int x=0,y=0;
        Type type=Type::Empty;
        Door()=default;
        Door(int px,int py):x(px),y(py){}
        void set(Type t){ if(static_cast<int>(t)>static_cast<int>(type)) type=t; }
    };

    Room()=default;
    Room(int l,int t,int r,int b):Rect(l,t,r,b){}

    std::vector<Room*> neighbours;
    std::unordered_map<Room*,std::shared_ptr<Door>> connected;
    float distance=0.0f;
    float price=1.0f;
    Type type=Type::Null;

    Point random(int margin=0) const;
    void addNeighbour(Room* other);
    bool connect(Room* other);
    bool insideCell(const Point& p) const;
    Point center() const;
    std::vector<std::shared_ptr<Door>> doors() const;
};
}
