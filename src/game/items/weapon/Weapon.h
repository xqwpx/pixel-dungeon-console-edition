#pragma once
#include "game/items/KindOfWeapon.h"
#include "game/items/weapon/Enchantment.h"
#include <memory>

namespace pdv {

class Weapon : public KindOfWeapon {
public:
    enum class Imbue { None, Speed, Accuracy };
    Weapon() = default;
    Weapon(const Weapon& other);
    Weapon& operator=(const Weapon& other);
    int STR() const { return strength_; }
    float ACU() const { return accuracy_; }
    float DLY() const { return delay_; }

    virtual bool isMissileWeapon() const { return false; }
    float accuracyFactor(const Hero& hero) const override;
    float speedFactor(const Hero& hero) const override;
    int damageRoll(const Hero& hero) const override;

    void enchant();
    void enchant(EnchantmentKind kind);
    void enchant(std::unique_ptr<Enchantment> enchantment);
    bool isEnchanted() const { return enchantment_ != nullptr; }
    int enchantmentId() const { return enchantment_ ? static_cast<int>(enchantment_->kind()) : 0; }
    Enchantment* enchantment() const { return enchantment_.get(); }
    void proc(Char& attacker, Char& defender, int damage) override;
    int maxDurability(int level) const override { return 5 * (level < 16 ? 16 - level : 1); }
    std::string displayName() const override;
    Imbue imbue()const{return imbue_;}
    void imbue(Imbue value){imbue_=value;}

protected:
    int strength_ = 10;
    float accuracy_ = 1.0f;
    float delay_ = 1.0f;
    std::unique_ptr<Enchantment> enchantment_;
    Imbue imbue_=Imbue::None;
};

} // namespace pdv
