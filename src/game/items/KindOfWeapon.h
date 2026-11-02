#pragma once
#include "game/items/Item.h"

namespace pdv {

class Char;
class Hero;

class KindOfWeapon : public Item {
public:
    std::vector<ItemAction> actions(const Hero& hero) const override;
    virtual int min() const = 0;
    virtual int max() const = 0;

    virtual int damageRoll(const Hero& owner) const;

    virtual float accuracyFactor(const Hero& hero) const {
        (void)hero;
        return 1.0f;
    }

    virtual float speedFactor(const Hero& hero) const {
        (void)hero;
        return 1.0f;
    }

    virtual void proc(Char& attacker, Char& defender, int damage) {
        (void)attacker;
        (void)defender;
        (void)damage;
    }
};

} // namespace pdv
