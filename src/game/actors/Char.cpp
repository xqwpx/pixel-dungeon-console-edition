#include "game/actors/Char.h"

#include "game/actors/buffs/Buff.h"
#include "game/actors/buffs/Charm.h"
#include "game/actors/buffs/Cripple.h"
#include "game/actors/buffs/Frost.h"
#include "game/actors/buffs/Slow.h"
#include "game/actors/buffs/Speed.h"
#include "game/actors/buffs/Vertigo.h"
#include "game/actors/buffs/GasesImmunity.h"
#include "game/actors/buffs/Paralysis.h"
#include "game/actors/blobs/SacrificialFire.h"
#include "game/levels/Level.h"
#include "game/utils/Random.h"

#include <algorithm>

namespace pdv {

Char::Char() = default;

Char::~Char() {
    clearBuffs();
}

float Char::speed() const {
    return hasBuff(typeid(Cripple))
        ? baseSpeed_ * 0.5f
        : baseSpeed_;
}

void Char::spend(float time) {
    float timeScale = 1.0f;

    if (hasBuff(typeid(Slow))) {
        timeScale *= 0.5f;
    }

    if (hasBuff(typeid(Speed))) {
        timeScale *= 2.0f;
    }

    Actor::spend(
        time / timeScale
    );
}


bool Char::attack(Char& enemy) {
    if (!hit(*this, enemy, false)) {
        return false;
    }

    const int absorbed =
        Random::IntRange(
            0,
            std::max(0, enemy.dr())
        );

    const int rolled = damageRoll();

    int effectiveDamage =
        std::max(
            rolled - absorbed,
            0
        );

    effectiveDamage =
        attackProc(
            enemy,
            effectiveDamage
        );

    effectiveDamage =
        enemy.defenseProc(
            *this,
            effectiveDamage
        );

    enemy.damage(
        effectiveDamage,
        this
    );

    return true;
}

bool Char::hit(
    const Char& attacker,
    const Char& defender,
    bool magic
) {
    const float acuRoll =
        Random::Float(
            static_cast<float>(
                std::max(
                    0,
                    attacker.attackSkill(defender)
                )
            )
        );

    const float defRoll =
        Random::Float(
            static_cast<float>(
                std::max(
                    0,
                    defender.defenseSkill(attacker)
                )
            )
        );

    return
        (magic ? acuRoll * 2.0f : acuRoll)
        >= defRoll;
}

int Char::attackProc(
    Char&,
    int damage
) {
    return damage;
}

int Char::defenseProc(
    Char&,
    int damage
) {
    return damage;
}

void Char::damage(
    int amount,
    const void* source
) {
    if (HP <= 0) {
        return;
    }

    // Original Char.damage(): any damage event first removes Frost.
    Buff::detach<Frost>(*this);

    const int damageDone =
        std::max(0, amount);

    // Original Char.damage(): enough pain can break Paralysis.
    if (hasBuff(typeid(Paralysis)) &&
        damageDone > 0) {

        if (Random::Int(damageDone) >=
            Random::Int(std::max(HP, 1))) {

            Buff::detach<Paralysis>(*this);
        }
    }

    HP -= damageDone;

    if (HP <= 0) {
        die(source);
    }
}

void Char::destroy() {
    HP = 0;

    if (ActorScheduler* s = scheduler()) {
        s->freeCell(pos);
        s->remove(this);
    }
}

void Char::die(const void*) {
    SacrificialFire::sacrifice(*this);
    Buff::detach<Marked>(*this);
    destroy();
}

bool Char::moveTo(
    int target,
    const Level& level
) {
    if (rooted) {
        return false;
    }

    if (!level.inBoundsCell(target) ||
        !Level::adjacent(pos, target)) {
        return false;
    }

    if (hasBuff(typeid(Vertigo))) {
        target =
            pos +
            Level::Neighbours8[
                static_cast<std::size_t>(
                    Random::Int(8)
                )
            ];

        if (!level.inBoundsCell(target)) {
            return false;
        }
    }

    if (!(level.passable(target) ||
          (flying && level.avoid(target)))) {
        return false;
    }

    ActorScheduler* s = scheduler();

    if (s) {
        Char* occupant =
            s->findChar(target);

        if (occupant != nullptr &&
            occupant != this) {
            return false;
        }

        s->freeCell(pos);
    }

    pos = target;

    if (s) {
        s->occupyCell(this);
    }

    return true;
}

int Char::distance(
    const Char& other
) const {
    return Level::distance(
        pos,
        other.pos
    );
}

Buff* Char::adoptBuff(
    std::unique_ptr<Buff> buff
) {
    if (!buff) {
        return nullptr;
    }

    Buff* raw = buff.get();

    if (!raw->attachTo(*this)) {
        return nullptr;
    }

    ownedBuffs_.push_back(
        std::move(buff)
    );

    return raw;
}

void Char::registerBuff(Buff* buff) {
    if (!buff ||
        std::find(
            buffs_.begin(),
            buffs_.end(),
            buff
        ) != buffs_.end()) {
        return;
    }

    buffs_.push_back(buff);

    if (ActorScheduler* s = scheduler()) {
        s->add(buff);
    }

    onBuffAdded(*buff);
}

void Char::unregisterBuff(Buff* buff) {
    if (!buff) {
        return;
    }

    const auto it =
        std::find(
            buffs_.begin(),
            buffs_.end(),
            buff
        );

    if (it == buffs_.end()) {
        return;
    }

    if (ActorScheduler* s = scheduler()) {
        s->remove(buff);
    }

    buffs_.erase(it);

    onBuffRemoved(*buff);
}

Buff* Char::findBuff(
    const std::type_info& type
) const {
    for (Buff* buff : buffs_) {
        if (buff &&
            buff->attached() &&
            typeid(*buff) == type) {
            return buff;
        }
    }

    return nullptr;
}

std::vector<Buff*> Char::activeBuffs() const {
    std::vector<Buff*> result;

    for (Buff* buff : buffs_) {
        if (buff && buff->attached()) {
            result.push_back(buff);
        }
    }

    return result;
}

void Char::clearBuffs() {
    const auto copy = buffs_;

    for (Buff* buff : copy) {
        if (buff && buff->attached()) {
            buff->detach();
        }
    }

    buffs_.clear();
    ownedBuffs_.clear();
}

bool Char::isCharmedBy(
    const Char& other
) const {
    const Buff* buff =
        findBuff(typeid(Charm));

    const Charm* charm =
        dynamic_cast<const Charm*>(
            buff
        );

    return
        charm &&
        charm->object ==
            const_cast<Char&>(other).id();
}

void Char::onRemove() {
    // Original Char.onRemove() detaches all buffs.
    const auto copy = buffs_;

    for (Buff* buff : copy) {
        if (buff && buff->attached()) {
            buff->detach();
        }
    }
}


bool Char::immuneTo(const std::type_info& type) const {
    if (hasBuff(typeid(GasesImmunity))) {
        return
            type == typeid(Paralysis) ||
            type == typeid(Vertigo);
    }

    return false;
}

} // namespace pdv
