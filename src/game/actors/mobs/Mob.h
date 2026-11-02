#pragma once

#include <array>
#include <string>

#include "game/actors/Char.h"
#include "game/levels/Level.h"

namespace pdv {

class Dungeon;
class Hero;
class Item;

class Mob : public Char {
public:
    enum class State {
        Sleeping,
        Wandering,
        Hunting,
        Fleeing,
        Passive
    };

    Mob();
    ~Mob() override = default;

    void bind(Dungeon* dungeon);

    bool act() override;

    State state() const { return state_; }
    void setState(State state) { state_ = state; }

    int target() const { return target_; }
    bool enemySeen() const { return enemySeen_; }
    bool hostile() const { return hostile_; }
    virtual bool isBoss()const{return false;}
    virtual int persistentState() const { return 0; }
    virtual void restorePersistentState(int) {}
    Char* enemy() const { return enemy_; }

    int defenseSkillValue() const { return defenseSkill_; }
    int expValue() const { return exp_; }
    int exp() const;
    int maxLevel() const { return maxLvl_; }

    int defenseSkill(const Char& enemy) const override;
    int defenseProc(Char& enemy, int damage) override;

    void damage(int amount, const void* source) override;
    void destroy() override;
    void die(const void* cause) override;

    virtual void beckon(int cell);
    void notice();

    virtual const char* description() const;

protected:
    void onBuffAdded(Buff& buff) override;
    void onBuffRemoved(Buff& buff) override;

protected:
    static constexpr float TimeToWakeUp = 1.0f;

    virtual bool canAttack(const Char& enemy) const;
    virtual float attackDelay() const { return 1.0f; }

    virtual bool doAttack(Char& enemy);
    virtual float lootChance() const { return 0.0f; }
    virtual std::unique_ptr<Item> createLoot() const;

    virtual bool getCloser(int target);
    virtual bool getFurther(int target);
    virtual void onMoved(int) {}

    bool actSleeping(bool enemyInFOV);
    bool actWandering(bool enemyInFOV, bool justAlerted);
    bool actHunting(bool enemyInFOV);
    virtual bool actFleeing(bool enemyInFOV);
    bool actPassive();

    int randomDestination() const;
    virtual Char* chooseEnemy();

    Dungeon* dungeon_ = nullptr;
    Char* enemy_ = nullptr;

    State state_ = State::Sleeping;

    int target_ = -1;

    int defenseSkill_ = 0;
    int exp_ = 1;
    int maxLvl_ = 30;

    bool enemySeen_ = false;
    bool alerted_ = false;
    bool hostile_ = true;
};

const char* mobStateName(Mob::State state);

} // namespace pdv
