#pragma once

#include <memory>
#include <typeinfo>

#include "game/actors/Actor.h"
#include "game/actors/Char.h"

namespace pdv {

class Buff : public Actor {
public:
    virtual ~Buff() = default;

    Char* target() const { return target_; }
    bool attached() const { return attached_; }

    virtual bool attachTo(Char& target);
    virtual void detach();

    virtual float persistentValue() const { return 0.0f; }
    virtual int persistentInt() const { return 0; }
    virtual void restorePersistent(float cooldownValue,float,int) {
        resetTime();
        spend(cooldownValue > 0.0f ? cooldownValue : 0.0f);
    }

    bool act() override;

    template<class T>
    static T* append(Char& target) {
        std::unique_ptr<T> buff(new T());
        T* raw = buff.get();

        if (!target.adoptBuff(std::move(buff))) {
            return nullptr;
        }

        return raw;
    }

    template<class T>
    static T* append(Char& target, float duration) {
        T* buff = append<T>(target);
        if (buff) {
            buff->spend(duration);
        }
        return buff;
    }

    template<class T>
    static T* affect(Char& target) {
        Buff* existing = target.findBuff(typeid(T));

        if (existing) {
            return dynamic_cast<T*>(existing);
        }

        return append<T>(target);
    }

    template<class T>
    static T* affect(Char& target, float duration) {
        T* buff = affect<T>(target);
        if (buff) {
            buff->spend(duration);
        }
        return buff;
    }

    template<class T>
    static T* prolong(Char& target, float duration) {
        T* buff = affect<T>(target);
        if (buff) {
            buff->postpone(duration);
        }
        return buff;
    }

    template<class T>
    static void detach(Char& target) {
        Buff* buff = target.findBuff(typeid(T));
        if (buff) {
            buff->detach();
        }
    }

protected:
    Char* target_ = nullptr;
    bool attached_ = false;
};

} // namespace pdv
