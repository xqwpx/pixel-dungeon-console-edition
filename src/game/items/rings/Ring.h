#pragma once
#include "game/items/Item.h"
#include <cstdint>
#include <limits>
namespace pdv {
class Char;
class Ring: public Item {
public:
    Ring(){ stackable_=false; }
    int effectiveLevel() const override { return Item::effectiveLevel(); }
    int maxDurability(int level) const override { return level <= 1 ? std::numeric_limits<int>::max() : 100 * (level < 16 ? 16 - level : 1); }
    virtual int stealthBonus() const { return 0; }
    virtual int accuracyBonus() const { return 0; }
    virtual int evasionBonus() const { return 0; }
    virtual int hasteBonus() const { return 0; }
    virtual int satietyBonus() const { return 0; }
    virtual int mendingBonus() const { return 0; }
    virtual int powerBonus() const { return 0; }
    virtual int detectionBonus() const { return 0; }
    // A level-zero Ring of Detection still grants its normal passive search
    // radius, so this cannot be inferred from detectionBonus() alone.
    virtual bool detectsSecrets() const { return false; }
    virtual int herbalismBonus() const { return 0; }
    virtual int elementsBonus() const { return 0; }
    virtual bool haggling() const { return false; }
    virtual bool thorns() const { return false; }
    virtual int statusIndex() const { return -1; }
    static void initGems();
    static uint16_t knownMask();
    static void restoreKnownMask(uint16_t mask);
    bool isKnown() const;
    void setKnown();
    Item& identify() override;
    bool isIdentified() const override;
    int image() const override;
    std::string displayName() const override;
    int price() const override { return priceWithState(80); }
    std::unique_ptr<Item> clone() const override = 0;
    std::vector<ItemAction> actions(const Hero& hero) const override;
};
float elementsDurationFactor(const Char& ch);
} // namespace pdv
