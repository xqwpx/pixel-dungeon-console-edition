#pragma once
#include <memory>
#include <cstdint>
#include "game/items/KindOfWeapon.h"
#include "game/actors/Actor.h"
namespace pdv {
class Dungeon; class Hero;
class Wand: public KindOfWeapon {
public:
    class Charger: public Actor {
    public:
        void bind(Wand* wand, Hero* hero);
        bool act() override;
        Hero* hero() const { return hero_; }
    private:
        Wand* wand_=nullptr; Hero* hero_=nullptr;
    };
    Wand();
    ~Wand() override;
    int min() const override;
    int max() const override;
    int maxCharges() const { return maxCharges_; }
    int curCharges() const { return curCharges_; }
    void restoreCharges(int current) { curCharges_ = current < 0 ? 0 : (current > maxCharges_ ? maxCharges_ : current); }

    bool recharge(bool full) {
        if (curCharges_ >= maxCharges_) {
            return false;
        }

        curCharges_ =
            full
            ? maxCharges_
            : curCharges_ + 1;

        return true;
    }
    int power() const;
    virtual int statusIndex() const { return -1; }
    static void initAppearances();
    static uint16_t knownMask();
    static void restoreKnownMask(uint16_t mask);
    bool isKnown() const;
    void setKnown();
    Item& identify() override;
    bool isIdentified() const override;
    int image() const override;
    std::string displayName() const override;
    int price() const override { return priceWithState(50); }
    virtual int initialCharges() const { return 2; }
    bool isUpgradable() const override { return true; }
    int maxDurability(int level) const override { return 6 * (level < 16 ? 16 - level : 1); }
    Item& upgrade(int n=1) override;
    Item& degrade(int n=1) override;
    void activate(Hero& hero, ActorScheduler& scheduler);
    void stopCharging();
    float rechargeDelay(const Hero& hero) const;
    bool zap(Dungeon& dungeon, Hero& hero, int target);
    virtual void onZap(Dungeon& dungeon, Hero& hero, int cell)=0;
    void updateLevel();
    std::unique_ptr<Item> clone() const override = 0;
    std::vector<ItemAction> actions(const Hero& hero) const override;
    std::string info() const override;
protected:
    bool hitChars_=true;
    int maxCharges_=2;
    int curCharges_=2;
    std::unique_ptr<Charger> charger_;
};
} // namespace pdv
