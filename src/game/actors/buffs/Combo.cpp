#include "game/actors/buffs/Combo.h"
#include "game/Progression.h"
#include <algorithm>

namespace pdv {
int Combo::hit(int damage) {
    ++count_;
    Progression::instance().noteCombo(count_);
    postpone(count_ >= 3 ? std::max(0.0f, 1.41f - count_ / 10.0f) : 1.1f);
    return count_ >= 3 ? static_cast<int>(damage * (count_ - 2) / 5.0f) : 0;
}
bool Combo::act() { detach(); return true; }
}
