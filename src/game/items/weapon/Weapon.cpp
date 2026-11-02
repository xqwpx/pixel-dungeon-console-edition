#include "game/items/weapon/Weapon.h"
#include "core/Localization.h"
#include "game/actors/Hero.h"
#include "game/utils/Random.h"
#include <cmath>

namespace pdv {

Weapon::Weapon(const Weapon& other) : KindOfWeapon(other), strength_(other.strength_), accuracy_(other.accuracy_), delay_(other.delay_),imbue_(other.imbue_) {
    if (other.enchantment_) enchantment_ = other.enchantment_->clone();
}
Weapon& Weapon::operator=(const Weapon& other) {
    if (this != &other) { KindOfWeapon::operator=(other); strength_=other.strength_; accuracy_=other.accuracy_; delay_=other.delay_;imbue_=other.imbue_; enchantment_=other.enchantment_ ? other.enchantment_->clone() : nullptr; }
    return *this;
}

float Weapon::accuracyFactor(const Hero& hero) const {
    int encumbrance = strength_ - hero.STR();
    if (isMissileWeapon()) {
        switch (hero.heroClass()) {
            case HeroClass::Warrior: encumbrance += 3; break;
            case HeroClass::Huntress: encumbrance -= 2; break;
            default: break;
        }
    }
    const float base=encumbrance > 0
        ? static_cast<float>(accuracy_ / std::pow(1.5, encumbrance))
        : accuracy_;return base*(imbue_==Imbue::Accuracy?1.5f:1.0f);
}

float Weapon::speedFactor(const Hero& hero) const {
    int encumbrance = strength_ - hero.STR();
    if (isMissileWeapon() && hero.heroClass() == HeroClass::Huntress) {
        encumbrance -= 2;
    }
    const float base=encumbrance > 0
        ? static_cast<float>(delay_ * std::pow(1.2, encumbrance))
        : delay_;return base*(imbue_==Imbue::Speed?0.6f:1.0f);
}

int Weapon::damageRoll(const Hero& hero) const {
    int damage = KindOfWeapon::damageRoll(hero);
    // Original Weapon.java: excess STR applies to melee non-Huntress,
    // and to ranged Huntress only.
    if ((hero.rangedWeapon() != nullptr) == (hero.heroClass() == HeroClass::Huntress)) {
        const int excess = hero.STR() - strength_;
        if (excess > 0) damage += Random::IntRange(0, excess);
    }
    return damage;
}



void Weapon::enchant() {
    const int old = enchantmentId();
    do { enchantment_ = Enchantment::random(); } while (enchantmentId() == old);
}
void Weapon::enchant(EnchantmentKind kind) { enchantment_ = Enchantment::create(kind); }
void Weapon::enchant(std::unique_ptr<Enchantment> enchantment) { enchantment_ = std::move(enchantment); }
void Weapon::proc(Char& attacker, Char& defender, int damage) { if (enchantment_) enchantment_->proc(*this, attacker, defender, damage); use(); }
std::string Weapon::displayName() const { return enchantment_ ? enchantment_->name(Item::displayName()) : Item::displayName(); }
} // namespace pdv
