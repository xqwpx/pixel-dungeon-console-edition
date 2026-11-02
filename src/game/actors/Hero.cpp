#include "game/actors/Hero.h"
#include "game/Progression.h"

#include "game/items/KindOfWeapon.h"
#include "game/items/armor/Armor.h"
#include "game/items/armor/ClothArmor.h"
#include "game/items/food/Food.h"
#include "game/items/bags/Keyring.h"
#include "game/actors/buffs/Hunger.h"
#include "game/actors/buffs/Combo.h"
#include "game/actors/buffs/Fury.h"
#include "game/actors/buffs/SnipersMark.h"
#include "game/actors/buffs/Barkskin.h"
#include "game/actors/buffs/HerbalArmor.h"
#include "game/items/weapon/melee/Dagger.h"
#include "game/items/weapon/melee/Knuckles.h"
#include "game/items/weapon/melee/ShortSword.h"
#include "game/items/weapon/melee/MeleeWeapon.h"
#include "game/items/weapon/missiles/Dart.h"
#include "game/items/weapon/missiles/Boomerang.h"
#include "game/items/wands/WandOfMagicMissile.h"
#include "game/items/wands/Wand.h"
#include "game/items/rings/RingOfShadows.h"
#include "game/items/rings/Ring.h"
#include "game/ui/QuickSlot.h"
#include "game/levels/Level.h"
#include "game/levels/RegularLevel.h"
#include "game/utils/Random.h"

#include <algorithm>
#include <cmath>
#include <memory>

namespace pdv {


Hero::Hero()
    : belongings_(this), primaryQuickSlot_(new QuickSlot()) {
}

void Hero::initialize(HeroClass cl, int startPos) {
    clearBuffs();
    resetTime();
    heroClass_ = cl;
    subClass_ = HeroSubClass::None;
    pos = startPos;

    name = "you";

    HT = 20;
    HP = HT;

    attackSkill_ = 10;
    defenseSkill_ = 5;
    strength_ = 10;

    weakened_ = false;

    level_ = 1;
    exp_ = 0;
    hpUpgrades_=strengthUpgrades_=enduranceUpgrades_=luckUpgrades_=0;
    awareness_ = 0.1f;

    viewDistance = Level::ViewDistance;

    flying = false;
    paralysed = false;
    rooted = false;
    invisible = 0;

    belongings_.reset(this);
    rangedWeapon_ = nullptr;
    primaryQuickSlot_->clear();
    initStartingEquipment();
    updateAwareness();
}

void Hero::initStartingEquipment() {
    // Original HeroClass.initCommon(): ClothArmor + Food + Keyring.
    std::unique_ptr<ClothArmor> armor(new ClothArmor());
    armor->identify();
    belongings_.equipArmor(std::move(armor));

    std::unique_ptr<Food> food(new Food());
    food->identify();
    belongings_.backpack().collect(std::move(food));
    belongings_.backpack().collect(std::unique_ptr<Item>(new Keyring()));

    switch (heroClass_) {
        case HeroClass::Warrior: {
            strength_ += 1;

            std::unique_ptr<ShortSword> weapon(new ShortSword());
            weapon->identify();
            belongings_.equipWeapon(std::move(weapon));
            std::unique_ptr<Dart> darts(new Dart(8));
            darts->identify();
            belongings_.backpack().collect(std::move(darts));
            primaryQuickSlot_->setClass(QuickSlotKind::Dart);
            break;
        }

        case HeroClass::Mage: {
            std::unique_ptr<Knuckles> weapon(new Knuckles());
            weapon->identify();
            belongings_.equipWeapon(std::move(weapon));
            std::unique_ptr<WandOfMagicMissile> wand(new WandOfMagicMissile());
            wand->identify();
            WandOfMagicMissile* quick = wand.get();
            belongings_.backpack().collect(std::move(wand));
            primaryQuickSlot_->setItem(QuickSlotKind::WandOfMagicMissile, quick);
            break;
        }

        case HeroClass::Rogue: {
            std::unique_ptr<Dagger> weapon(new Dagger());
            weapon->identify();
            belongings_.equipWeapon(std::move(weapon));
            std::unique_ptr<RingOfShadows> ring(new RingOfShadows());
            ring->upgrade();
            ring->identify();
            belongings_.equipRing(std::move(ring));
            std::unique_ptr<Dart> darts(new Dart(8));
            darts->identify();
            belongings_.backpack().collect(std::move(darts));
            primaryQuickSlot_->setClass(QuickSlotKind::Dart);
            break;
        }

        case HeroClass::Huntress: {
            HP = (HT -= 5);

            std::unique_ptr<Dagger> weapon(new Dagger());
            weapon->identify();
            belongings_.equipWeapon(std::move(weapon));
            std::unique_ptr<Boomerang> boomerang(new Boomerang());
            boomerang->identify();
            Boomerang* quick = boomerang.get();
            belongings_.backpack().collect(std::move(boomerang));
            primaryQuickSlot_->setItem(QuickSlotKind::Boomerang, quick);
            break;
        }
    }

}

bool Hero::act() {
    return false;
}

int Hero::attackSkill(const Char& target) const {
    int ringBonus=0;for(const Ring* ring:{belongings_.ring1(),belongings_.ring2()})if(ring)ringBonus+=ring->accuracyBonus();
    float accuracy = ringBonus==0?1.0f:static_cast<float>(std::pow(1.4,ringBonus));
    if (rangedWeapon_ != nullptr && Level::distance(pos, target.pos) == 1) accuracy *= 0.5f;
    const KindOfWeapon* weapon = rangedWeapon_ ? rangedWeapon_ : belongings_.weapon();
    if (weapon) accuracy *= weapon->accuracyFactor(*this);
    return static_cast<int>(static_cast<float>(attackSkill_ + strengthUpgrades_ * 2) * accuracy);
}

int Hero::defenseSkill(const Char&) const {
    int ringBonus=0;for(const Ring* ring:{belongings_.ring1(),belongings_.ring2()})if(ring)ringBonus+=ring->evasionBonus();
    float evasion = ringBonus==0?1.0f:static_cast<float>(std::pow(1.2,ringBonus));

    if (paralysed) {
        evasion *= 0.5f;
    }

    const Armor* armor = belongings_.armor();

    const int aEnc = armor
        ? armor->STR() - STR()
        : 0;

    if (aEnc > 0) {
        return static_cast<int>(
            static_cast<float>(defenseSkill_) *
            evasion /
            std::pow(1.5, aEnc)
        );
    }

    if (heroClass_ == HeroClass::Rogue) {
        if (subClass_ == HeroSubClass::Freerunner && !isStarving() && aEnc <= 0) {
            evasion *= 2.0f;
        }
        return static_cast<int>(
            static_cast<float>(defenseSkill_ - aEnc) * evasion
        );
    }

    return static_cast<int>(
        static_cast<float>(defenseSkill_) * evasion
    );
}

int Hero::dr() const {
    const Armor* armor = belongings_.armor();
    int value=armor ? std::max(armor->DR(), 0) : 0;
    if(const Barkskin* bark=dynamic_cast<const Barkskin*>(findBuff(typeid(Barkskin))))value+=bark->level();
    return value;
}

int Hero::damageRoll() const {
    const KindOfWeapon* weapon = rangedWeapon_ ? rangedWeapon_ : belongings_.weapon();

    int damage = weapon ? weapon->damageRoll(*this) : (STR() > 10
        ? Random::IntRange(1, STR() - 9)
        : 1);
    damage += strengthUpgrades_;
    return hasBuff(typeid(Fury)) ? static_cast<int>(damage * 1.5f) : damage;
}

int Hero::attackProc(Char& enemy, int damage) {
    if (KindOfWeapon* weapon = rangedWeapon_ ? rangedWeapon_ : belongings_.weapon()) {
        weapon->proc(*this, enemy, damage);
        if (subClass_ == HeroSubClass::Gladiator &&
            dynamic_cast<MeleeWeapon*>(weapon)) {
            if (Combo* combo = Buff::affect<Combo>(*this)) damage += combo->hit(damage);
        }
        if (subClass_ == HeroSubClass::Battlemage) {
            if (Wand* wand = dynamic_cast<Wand*>(weapon)) {
                if (damage > 0 && wand->curCharges() < wand->maxCharges()) wand->recharge(false);
                damage += wand->curCharges();
            }
        }
        if (subClass_ == HeroSubClass::Sniper && rangedWeapon_) {
            if (SnipersMark* mark=Buff::prolong<SnipersMark>(*this,attackDelay()*1.1f))mark->object=enemy.id();
        }
    }

    return damage;
}

void Hero::damage(int amount, const void* source) {
    if (invulnerable_ && amount > 0) return;
    // A small global cushion makes regular encounters forgiving without
    // changing enemy AI or boss mechanics.  One-point hits stay meaningful.
    if (amount > 1) amount = std::max(1, (amount * 3 + 3) / 4);
    Char::damage(amount, source);
    if (subClass_ == HeroSubClass::Berserker && HP > 0 && HP <= HT * Fury::Level) {
        Buff::affect<Fury>(*this);
    }
}

int Hero::defenseProc(Char& enemy, int damage) {
    for(const Ring*ring:{belongings_.ring1(),belongings_.ring2()})if(ring&&ring->thorns()&&damage>0){const int reflected=Random::IntRange(0,damage);if(reflected>0){enemy.damage(reflected,ring);Progression::instance().noteThorns();}break;}
    if(HerbalArmor* herbal=dynamic_cast<HerbalArmor*>(findBuff(typeid(HerbalArmor))))damage=herbal->absorb(damage);
    if (Armor* armor = belongings_.armor()) {
        damage = armor->proc(enemy, *this, damage);
    }

    return damage;
}

float Hero::speed() const {
    const Armor* armor = belongings_.armor();

    const int aEnc = armor
        ? armor->STR() - STR()
        : 0;

    if (aEnc > 0) {
        return static_cast<float>(
            Char::speed() * std::pow(1.3, -aEnc)
        );
    }

    const float speed = Char::speed();
    return subClass_ == HeroSubClass::Freerunner && !isStarving()
        ? speed * 1.6f
        : speed;
}

float Hero::attackDelay() const {
    const KindOfWeapon* weapon = rangedWeapon_ ? rangedWeapon_ : belongings_.weapon();
    return weapon ? weapon->speedFactor(*this) : 1.0f;
}

void Hero::spend(float time) {
    int bonus=0;for(const Ring* ring:{belongings_.ring1(),belongings_.ring2()})if(ring)bonus+=ring->hasteBonus();
    const float elapsed=bonus==0?time:static_cast<float>(time*std::pow(1.1,-bonus));
    Progression::instance().noteTime(elapsed);
    Char::spend(elapsed);
}

void Hero::spendMoveAndNext() {
    spend(Actor::TICK / speed());
    next();
}

void Hero::spendAttackAndNext() {
    spend(attackDelay());
    next();
}


void Hero::spendAndNext(float time) { spend(time); next(); }

bool Hero::eatFood() {
    return eatFood(
        belongings_
            .getItem<Food>()
    );
}

bool Hero::eatFood(Food* food) {
    if (!food) {
        return false;
    }

    std::unique_ptr<Item> detached =
        belongings_.detach(food);

    Food* ration =
        dynamic_cast<Food*>(
            detached.get()
        );

    if (!ration) {
        return false;
    }

    return ration->eat(*this);
}


bool Hero::isStarving() const {
    return hunger_ &&
           hunger_->isStarving();
}

void Hero::earnExp(int amount) {
    exp_ += std::max(0, amount);

    while (exp_ >= maxExp()) {
        exp_ -= maxExp();
        level_++;
        Progression::instance().noteHeroLevel(level_);

        HT += 5;
        HP += 5;

        attackSkill_++;
        defenseSkill_++;

        if (level_ < 10) {
            updateAwareness();
        }
    }

    if (subClass_ == HeroSubClass::Warlock) {
        const RegularLevel* level = dynamic_cast<const RegularLevel*>(levelContext());
        const int depth = level ? level->depth() : 1;
        HP = std::min(HT, HP + 1 + (depth - 1) / 5);
        if (hunger_) hunger_->satisfy(10.0f);
    }
}

int Hero::maxExp() const {
    return 5 + level_ * 5;
}

bool Hero::spendUpgrade(int stat){
    if(upgradePoints()<=0)return false;
    if(stat==0){++hpUpgrades_;HT+=5;HP=std::min(HT,HP+5);}
    else if(stat==1){++strengthUpgrades_;}
    else if(stat==2){++enduranceUpgrades_;belongings_.backpack().setSize(backpackCapacity());}
    else if(stat==3){++luckUpgrades_;}
    else return false;
    return true;
}

int Hero::backpackCapacity() const {
    return Belongings::BackpackSize + enduranceUpgrades_ * 3;
}

void Hero::updateAwareness() {
    const double base =
        heroClass_ == HeroClass::Rogue
        ? 0.85
        : 0.90;

    awareness_ = static_cast<float>(
        1.0 -
        std::pow(
            base,
            (1 + std::min(level_, 9)) * 0.5
        )
    );
}


int Hero::stealth() const {
    int value = Char::stealth();
    if (const Ring* ring = belongings_.ring1()) value += ring->stealthBonus();
    if (const Ring* ring = belongings_.ring2()) value += ring->stealthBonus();
    return value;
}

bool Hero::shoot(Char& enemy, KindOfWeapon& weapon) {
    rangedWeapon_ = &weapon;
    const bool result = attack(enemy);
    rangedWeapon_ = nullptr;
    return result;
}

QuickSlot& Hero::primaryQuickSlot(){ return *primaryQuickSlot_; }
const QuickSlot& Hero::primaryQuickSlot() const { return *primaryQuickSlot_; }

} // namespace pdv
