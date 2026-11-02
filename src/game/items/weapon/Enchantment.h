#pragma once
#include <memory>
#include <string>

namespace pdv {
class Char;
class Weapon;

enum class EnchantmentKind {
    Fire = 1, Poison, Death, Paralysis, Leech, Slow,
    Shock, Instability, Horror, Luck, Tempering
};

class Enchantment {
public:
    virtual ~Enchantment() = default;
    virtual EnchantmentKind kind() const = 0;
    virtual bool proc(Weapon& weapon, Char& attacker, Char& defender, int damage) = 0;
    virtual std::string name(const std::string& weaponName) const = 0;
    virtual std::unique_ptr<Enchantment> clone() const = 0;

    static std::unique_ptr<Enchantment> create(EnchantmentKind kind);
    static std::unique_ptr<Enchantment> random();
    static int weight(EnchantmentKind kind);
};
} // namespace pdv
