#pragma once
#include "game/actors/buffs/FlavourBuff.h"
namespace pdv {
class SnipersMark : public FlavourBuff {
public:
    int object = 0;
    int persistentInt() const override { return object; }
    void restorePersistent(float c,float v,int s) override { object=s;FlavourBuff::restorePersistent(c,v,s); }
};
}
