#pragma once

#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

#include "game/actors/Actor.h"

namespace pdv {

class Buff;
class Level;

class Char : public Actor {
public:
    Char();
    ~Char() override;

    int pos = 0;

    int HP = 1;
    int HT = 1;

    int viewDistance = 8;

    bool paralysed = false;
    bool rooted = false;
    bool flying = false;

    int invisible = 0;

    std::string name = "mob";

    bool isAlive() const { return HP > 0; }

    virtual float speed() const;
    void spend(float time) override;
    virtual int stealth() const { return 0; }

    virtual int attackSkill(const Char&) const { return 0; }
    virtual int defenseSkill(const Char&) const { return 0; }
    virtual int dr() const { return 0; }
    virtual int damageRoll() const { return 1; }

    virtual int attackProc(Char& enemy, int damage);
    virtual int defenseProc(Char& enemy, int damage);

    bool attack(Char& enemy);
    static bool hit(const Char& attacker, const Char& defender, bool magic = false);

    virtual void damage(int amount, const void* source);
    virtual void destroy();
    virtual void die(const void* source);

    bool moveTo(int target, const Level& level);
    int distance(const Char& other) const;

    void bindLevel(Level* level) { levelContext_ = level; }
    Level* levelContext() const { return levelContext_; }

    Buff* adoptBuff(std::unique_ptr<Buff> buff);
    void registerBuff(Buff* buff);
    void unregisterBuff(Buff* buff);

    Buff* findBuff(const std::type_info& type) const;
    bool hasBuff(const std::type_info& type) const {
        return findBuff(type) != nullptr;
    }

    std::vector<Buff*> activeBuffs() const;
    void clearBuffs();

    virtual bool immuneTo(const std::type_info& type) const;
    virtual bool resistantTo(const std::type_info&) const { return false; }
    virtual const char* defenseVerb() const { return "dodged"; }

    bool isCharmedBy(const Char& other) const;

protected:
    virtual void onBuffAdded(Buff&) {}
    virtual void onBuffRemoved(Buff&) {}

    void onRemove() override;

    float baseSpeed_ = 1.0f;

private:
    std::vector<Buff*> buffs_;
    std::vector<std::unique_ptr<Buff>> ownedBuffs_;

    Level* levelContext_ = nullptr;
};

} // namespace pdv
