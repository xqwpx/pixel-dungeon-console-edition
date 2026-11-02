#pragma once
#include "game/actors/mobs/Mob.h"
#include <memory>
#include <vector>
namespace pdv {
class Item;
class Mimic:public Mob{
public:
    Mimic();
    void adjustStats(int level);
    int level()const{return level_;}
    void addItem(std::unique_ptr<Item> item);
    const std::vector<std::unique_ptr<Item>>&items()const{return items_;}
    int damageRoll()const override;
    int attackSkill(const Char&)const override;
    int attackProc(Char&enemy,int damage)override;
    void die(const void*cause)override;
    const char*description()const override;
private:int level_=1;std::vector<std::unique_ptr<Item>>items_;
};
}
