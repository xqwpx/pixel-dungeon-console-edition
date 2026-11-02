#pragma once

#include "game/actors/mobs/Mob.h"

namespace pdv {

enum class ZoneMobKind {
    Shaman, Bat, Brute, Spinner, Elemental, Monk, Golem, Warlock, Succubus, Eye,
    Scorpio, Bandit, Shielded, Senior, Acidic, Undead, RottingFist, BurningFist, Larva, Sheep
};

class ZoneMob : public Mob {
public:
    explicit ZoneMob(ZoneMobKind kind);

    ZoneMobKind kind() const { return kind_; }
    int persistentState() const override { return kind_ == ZoneMobKind::Succubus ? blinkDelay_ : (kind_ == ZoneMobKind::Sheep ? life_ : 0); }
    void restorePersistentState(int value) override { if (kind_ == ZoneMobKind::Succubus) blinkDelay_ = value; else if (kind_ == ZoneMobKind::Sheep) life_ = value; }
    void setSheepLife(int value) { if (kind_ == ZoneMobKind::Sheep) life_ = value; }

    bool act() override;
    void damage(int, const void*) override;
    void die(const void*) override;
    int damageRoll() const override;
    int attackSkill(const Char&) const override;
    int dr() const override;
    int attackProc(Char&, int) override;
    int defenseProc(Char&, int) override;
    bool immuneTo(const std::type_info&) const override;
    bool resistantTo(const std::type_info&) const override;
    const char* description() const override;

protected:
    bool canAttack(const Char&) const override;
    bool doAttack(Char&) override;
    bool getCloser(int) override;
    float attackDelay() const override;
    float lootChance() const override;
    std::unique_ptr<Item> createLoot() const override;
    void onMoved(int) override;
    void onBuffAdded(Buff&) override;

private:
    ZoneMobKind kind_;
    int minDamage_ = 1, maxDamage_ = 4, skill_ = 10, dr_ = 0;
    bool ranged_ = false;
    int life_ = 0, blinkDelay_ = 0;
};

enum class ZoneBossKind { Tengu, DM300, King, Yog };

class ZoneBoss : public Mob {
public:
    explicit ZoneBoss(ZoneBossKind kind);

    ZoneBossKind kind() const { return kind_; }
    bool isBoss() const override { return true; }
    bool act() override;
    int damageRoll() const override;
    int attackSkill(const Char&) const override;
    int dr() const override;
    int attackProc(Char&, int) override;
    int defenseProc(Char&, int) override;
    void damage(int, const void*) override;
    void die(const void*) override;
    void beckon(int) override;
    bool immuneTo(const std::type_info&) const override;
    bool resistantTo(const std::type_info&) const override;
    const char* description() const override;
    bool sealed() const { return sealed_; }
    bool summonsSpawned() const { return summonsSpawned_; }
    void sealArena() { sealed_ = true; }
    void activateYogArena() { if (kind_ == ZoneBossKind::Yog) { sealArena(); summonsSpawned_ = true; } }
    int jumpDelay() const { return kind_ == ZoneBossKind::King ? (nextPedestal_ ? 1 : 0) : jumpDelay_; }
    void restoreBossState(bool sealed, bool summoned, int timer) { sealed_ = sealed; summonsSpawned_ = summoned; if (kind_ == ZoneBossKind::King) nextPedestal_ = timer != 0; else jumpDelay_ = timer; }

protected:
    bool canAttack(const Char&) const override;
    bool doAttack(Char&) override;
    bool getCloser(int) override;
    void onMoved(int) override;
    float lootChance() const override;
    std::unique_ptr<Item> createLoot() const override;

private:
    void spawnMinion(ZoneMobKind, int);
    bool tenguJump();
    bool kingCanSummon() const;
    int kingPedestal() const;

    ZoneBossKind kind_;
    int minDamage_ = 8, maxDamage_ = 15, skill_ = 20, dr_ = 5;
    bool sealed_ = false, summonsSpawned_ = false, nextPedestal_ = true;
    int jumpDelay_ = 5;
};

} // namespace pdv
