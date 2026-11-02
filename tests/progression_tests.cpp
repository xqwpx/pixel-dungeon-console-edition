#include <cassert>
#include <cstdio>
#include <iostream>
#include "game/Progression.h"
using namespace pdv;
int main(){
    const char* path="profile_test.dat";
    std::remove(path);
    Progression& p=Progression::instance();p.resetForTests();p.beginRun(HeroClass::Warrior);p.noteDepth(2);assert(p.run().completedWithNoKilling);p.noteKill(false);p.noteDepth(3);assert(!p.run().completedWithNoKilling);
    p.resetForTests();p.beginRun(HeroClass::Mage);
    p.noteDepth(25);p.noteHeroLevel(1);p.noteTime(12.5f);p.noteKill(true);p.noteKill(true);p.noteKill(true,HeroSubClass::Warlock);p.noteGold(1200);for(int i=0;i<50;++i)p.noteItem();p.finishRun(true);p.save(path);
    assert(p.runs()==1&&p.wins()==1&&p.deepest()==25&&p.rankings()[0].won&&p.rankings()[0].score==7400);
    assert(p.hasBadge(Badge::Victory)&&p.hasBadge(Badge::DungeonMaster)&&p.hasBadge(Badge::BossSlayer)&&p.hasBadge(Badge::Boss3Warlock));
    assert(p.run().duration==12.5f&&p.badgeName(Badge::Champion)=="Champion");
    p.resetForTests();p.load(path);assert(p.runs()==1&&p.wins()==1&&p.rankings()[0].score>0&&p.rankings()[0].duration==12.5f);
    std::remove(path);std::cout<<"PIXEL DUNGEON CONSOLE EDITION progression tests: OK\n";
}
