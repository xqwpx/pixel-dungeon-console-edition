#include "game/items/weapon/melee/MeleeWeapon.h"

namespace pdv {

MeleeWeapon::MeleeWeapon(int tier, float accuracy, float delay)
    : tier_(tier) {

    accuracy_ = accuracy;
    delay_ = delay;
    strength_ = typicalSTR();
}

int MeleeWeapon::min0() const {
    return tier_;
}

int MeleeWeapon::max0() const {
    return static_cast<int>(
        (tier_ * tier_ - tier_ + 10) / accuracy_ * delay_
    );
}

int MeleeWeapon::min() const {
    return min0() + level_;
}

int MeleeWeapon::max() const {
    return max0() + level_ * tier_;
}

int MeleeWeapon::typicalSTR() const {
    return 8 + tier_ * 2;
}

int MeleeWeapon::price() const {
    int value=20*(1<<(tier_-1));
    if (enchantmentId()!=0) value=value*3/2;
    return priceWithState(value);
}

} // namespace pdv
