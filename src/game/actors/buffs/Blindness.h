#pragma once
#include "game/actors/buffs/FlavourBuff.h"

namespace pdv {

class Blindness : public FlavourBuff {
public:
    void detach() override;
};

} // namespace pdv
