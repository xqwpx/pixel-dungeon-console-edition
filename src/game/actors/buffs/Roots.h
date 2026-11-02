#pragma once
#include "game/actors/buffs/FlavourBuff.h"

namespace pdv {

class Roots : public FlavourBuff {
public:
    bool attachTo(Char& target) override;
    void detach() override;
};

} // namespace pdv
