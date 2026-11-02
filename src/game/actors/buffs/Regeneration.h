#pragma once
#include "game/actors/buffs/Buff.h"

namespace pdv {

class Regeneration : public Buff {
public:
    static constexpr float RegenerationDelay = 10.0f;

    bool act() override;
};

} // namespace pdv
