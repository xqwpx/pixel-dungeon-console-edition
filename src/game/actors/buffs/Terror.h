#pragma once
#include "game/actors/buffs/FlavourBuff.h"

namespace pdv {

class Terror : public FlavourBuff {
public:
    static constexpr float Duration = 10.0f;

    int object = 0;

    static void recover(Char& target);
};

} // namespace pdv
