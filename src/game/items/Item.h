#pragma once
#include <memory>
#include <string>
#include <vector>
#include <algorithm>

namespace pdv {
class Hero;
enum class ItemAction { Equip, Unequip, Drop, Throw, Use, Drink, Read, Eat, Zap, QuickSlot,
    Gladiator, Berserker, Battlemage, Warlock, Assassin, Freerunner, Sniper, Warden,
    HeroicLeap, MoltenEarth, SmokeBomb, SpectralBlades, BalanceSpeed, BalanceAccuracy, Mine, Shatter };
const char* itemActionName(ItemAction action);
class Item {
public:
    static constexpr float TimeToPickUp = 1.0f;
    static constexpr float TimeToDrop = 0.5f;
    static constexpr float TimeToThrow = 1.0f;

    virtual ~Item() = default;
    const std::string& name() const { return name_; }
    virtual std::string displayName() const;
    virtual int image() const { return image_; }
    bool stackable() const { return stackable_; }
    int quantity() const { return quantity_; }
    void quantity(int value) { quantity_ = value; }
    int level() const { return level_; }
    void level(int value) { level_ = value; fix(); }
    virtual int effectiveLevel() const { return isBroken() ? 0 : level_; }
    virtual Item& upgrade(int n=1){ level_ += n; cursed_=false; fix(); return *this; }
    virtual Item& degrade(int n=1){ level_ -= n; fix(); return *this; }
    bool levelKnown() const { return levelKnown_; }
    void setLevelKnown(bool value) { levelKnown_ = value; }
    bool cursed() const { return cursed_; }
    void setCursed(bool value) { cursed_ = value; }
    bool cursedKnown() const { return cursedKnown_; }
    void setCursedKnown(bool value) { cursedKnown_ = value; }
    bool unique() const { return unique_; }

    virtual Item& identify();
    virtual bool isIdentified() const { return levelKnown_ && cursedKnown_; }
    virtual bool isUpgradable() const { return true; }
    virtual bool sameKind(const Item& other) const;
    virtual std::unique_ptr<Item> clone() const = 0;
    virtual std::vector<ItemAction> actions(const Hero& hero) const;
    virtual std::string info() const;
    virtual int price()const{return std::max(1,quantity_*(20+std::max(0,level_)*10));}
    virtual int maxDurability(int) const { return 1; }
    int maxDurability() const { return maxDurability(level_); }
    int durability() const { return durability_ < 0 ? maxDurability() : durability_; }
    bool isBroken() const { return durability() <= 0; }
    void use();
    void fix() { durability_ = maxDurability(); }
    void polish() { if (durability() < maxDurability()) ++durability_; }
    void restoreDurability(int value) { durability_ = std::max(0, std::min(maxDurability(), value)); }

protected:
    int priceWithState(int basePrice) const;
    std::string name_ = "smth";
    // Index 0 is BONES; unassigned items use the neutral marker.
    int image_ = 127;
    bool stackable_ = false;
    int quantity_ = 1;
    int level_ = 0;
    bool levelKnown_ = false;
    bool cursed_ = false;
    bool cursedKnown_ = false;
    bool unique_ = false;
    int durability_ = -1;
};
} // namespace pdv
