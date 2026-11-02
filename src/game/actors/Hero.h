#pragma once
#include "core/GameSession.h"
#include "game/actors/Belongings.h"
#include "game/actors/HeroSubClass.h"
#include "game/actors/Char.h"
#include "game/ui/QuickSlot.h"
#include <memory>
#include <algorithm>

namespace pdv {

class Hunger; class KindOfWeapon;

class Hero : public Char {
public:
    Hero();

    void initialize(HeroClass heroClass, int pos);
    bool act() override;

    int attackSkill(const Char& target) const override;
    int defenseSkill(const Char& enemy) const override;
    int dr() const override;
    int damageRoll() const override;
    int stealth() const override;

    bool shoot(Char& enemy, KindOfWeapon& weapon);
    KindOfWeapon* rangedWeapon() const { return rangedWeapon_; }

    int attackProc(Char& enemy, int damage) override;
    int defenseProc(Char& enemy, int damage) override;
    void damage(int amount, const void* source) override;
    bool invulnerable() const { return invulnerable_; }
    void setInvulnerable(bool value) { invulnerable_ = value; }

    float speed() const override;
    void spend(float time) override;
    float attackDelay() const;

    void spendMoveAndNext();
    void spendAttackAndNext();
    void spendAndNext(float time);

    void bindHunger(Hunger* hunger) { hunger_ = hunger; }
    Hunger* hunger() const { return hunger_; }
    bool eatFood();
    bool eatFood(class Food* food);
    bool isStarving() const;
    void setWeakened(bool value) { weakened_ = value; }

    void earnExp(int amount);
    void clearExperience() { exp_ = 0; }
    int maxExp() const;
    void updateAwareness();

    HeroClass heroClass() const { return heroClass_; }
    HeroSubClass subClass() const { return subClass_; }
    void setSubClass(HeroSubClass value) { subClass_ = value; }

    void increaseStrength(int value = 1) { strength_ += value; }

    int STR() const {
        return weakened_ ? strength_ - 2 : strength_;
    }
    int baseStrength() const { return strength_; }

    int level() const { return level_; }
    int exp() const { return exp_; }
    int upgradePoints() const { return std::max(0,level_-1-hpUpgrades_-strengthUpgrades_-enduranceUpgrades_-luckUpgrades_); }
    int hpUpgrades() const { return hpUpgrades_; }
    int strengthUpgrades() const { return strengthUpgrades_; }
    int enduranceUpgrades() const { return enduranceUpgrades_; }
    int luckUpgrades() const { return luckUpgrades_; }
    int backpackCapacity() const;
    bool spendUpgrade(int stat);
    float awareness() const { return awareness_; }

    void restoreProgress(int strength, int level, int exp) {
        strength_ = strength;
        level_ = level;
        exp_ = exp;
        updateAwareness();
    }
    void restoreUpgrades(int hp,int strength,int endurance,int luck){hpUpgrades_=hp;strengthUpgrades_=strength;enduranceUpgrades_=endurance;luckUpgrades_=luck;belongings_.backpack().setSize(backpackCapacity());}

    Belongings& belongings() { return belongings_; }
    const Belongings& belongings() const { return belongings_; }
    QuickSlot& primaryQuickSlot();
    const QuickSlot& primaryQuickSlot() const;

private:
    void initStartingEquipment();

    HeroClass heroClass_ = HeroClass::Warrior;
    HeroSubClass subClass_ = HeroSubClass::None;

    int attackSkill_ = 10;
    int defenseSkill_ = 5;

    int strength_ = 10;
    bool weakened_ = false;
    bool invulnerable_ = false;

    float awareness_ = 0.1f;

    int level_ = 1;
    int exp_ = 0;
    int hpUpgrades_ = 0;
    int strengthUpgrades_ = 0;
    int enduranceUpgrades_ = 0;
    int luckUpgrades_ = 0;

    Belongings belongings_;
    Hunger* hunger_ = nullptr;
    KindOfWeapon* rangedWeapon_ = nullptr;
    std::unique_ptr<QuickSlot> primaryQuickSlot_;
};

} // namespace pdv
