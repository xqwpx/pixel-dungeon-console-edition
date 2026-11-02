#include "game/actors/buffs/Buff.h"

namespace pdv {

bool Buff::attachTo(Char& target) {
    if (target.immuneTo(typeid(*this))) {
        return false;
    }

    target_ = &target;
    attached_ = true;
    target.registerBuff(this);

    return true;
}

void Buff::detach() {
    if (!attached_) {
        return;
    }

    Char* oldTarget = target_;

    attached_ = false;
    target_ = nullptr;

    if (oldTarget) {
        oldTarget->unregisterBuff(this);
    }
}

bool Buff::act() {
    deactivate();
    next();
    return true;
}

} // namespace pdv
