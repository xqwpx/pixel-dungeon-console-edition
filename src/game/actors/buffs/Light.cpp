#include "game/actors/buffs/Light.h"
#include "game/levels/Level.h"

#include <algorithm>

namespace pdv {
constexpr int Light::Distance;

bool Light::attachTo(Char& target) {
    if (!FlavourBuff::attachTo(target)) {
        return false;
    }

    if (Level* level = target.levelContext()) {
        target.viewDistance =
            std::max(
                level->viewDistance(),
                Distance
            );
    }

    return true;
}

void Light::detach() {
    Char* ch = target();

    if (ch) {
        if (Level* level = ch->levelContext()) {
            ch->viewDistance =
                level->viewDistance();
        }
    }

    FlavourBuff::detach();
}

} // namespace pdv
