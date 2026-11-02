#include "core/GameSession.h"

namespace pdv {

const char* heroClassName(HeroClass heroClass) {
    switch (heroClass) {
        case HeroClass::Warrior:  return "Warrior";
        case HeroClass::Mage:     return "Mage";
        case HeroClass::Rogue:    return "Rogue";
        case HeroClass::Huntress: return "Huntress";
    }

    return "Unknown";
}

} // namespace pdv
