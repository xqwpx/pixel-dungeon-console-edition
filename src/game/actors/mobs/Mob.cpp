#include "game/actors/mobs/Mob.h"
#include "core/StringUtils.h"
#include "game/actors/mobs/Albino.h"
#include "game/actors/mobs/Thief.h"
#include "game/actors/mobs/ZoneMob.h"

#include "game/Dungeon.h"
#include "game/Progression.h"
#include "game/actors/Hero.h"
#include "game/actors/buffs/Amok.h"
#include "game/actors/buffs/Sleep.h"
#include "game/actors/buffs/Charm.h"
#include "game/actors/buffs/Terror.h"
#include "game/utils/Random.h"
#include "game/items/Item.h"
#include <algorithm>

namespace pdv {

Mob::Mob() {
    name = "mob";
}

void Mob::bind(Dungeon* dungeon) {
    dungeon_ = dungeon;
    bindLevel(dungeon ? &dungeon->level() : nullptr);
}

bool Mob::act() {
    if (!dungeon_ ||
        !isAlive()) {
        spend(TICK);
        next();
        return true;
    }

    Level& level = dungeon_->level();

    level.updateFieldOfViewFor(
        *this
    );

    const bool justAlerted = alerted_;
    alerted_ = false;

    if (paralysed) {
        enemySeen_ = false;
        spend(TICK);
        next();
        return true;
    }

    enemy_ = chooseEnemy();

    const bool enemyInFOV =
        enemy_ != nullptr &&
        enemy_->isAlive() &&
        level.fieldOfView()[static_cast<std::size_t>(enemy_->pos)] &&
        enemy_->invisible <= 0;

    switch (state_) {
        case State::Sleeping:
            return actSleeping(enemyInFOV);

        case State::Wandering:
            return actWandering(enemyInFOV, justAlerted);

        case State::Hunting:
            return actHunting(enemyInFOV);

        case State::Fleeing:
            return actFleeing(enemyInFOV);

        case State::Passive:
            return actPassive();
    }

    return true;
}


Char* Mob::chooseEnemy() {
    if (!dungeon_) {
        return nullptr;
    }

    Level& level =
        dungeon_->level();

    if (hasBuff(typeid(Amok))) {
        if (enemy_ == &dungeon_->hero() ||
            enemy_ == nullptr ||
            !enemy_->isAlive()) {

            std::vector<Mob*> candidates;

            for (auto& mob : level.mobsMutable()) {
                if (!mob ||
                    mob.get() == this ||
                    !mob->isAlive() ||
                    !level.inBoundsCell(
                        mob->pos
                    )) {
                    continue;
                }

                if (level.fieldOfView()[
                        static_cast<std::size_t>(
                            mob->pos
                        )
                    ]) {
                    candidates.push_back(
                        mob.get()
                    );
                }
            }

            if (!candidates.empty()) {
                return Random::element(
                    candidates
                );
            }
        }
    }

    if (Terror* terror =
            dynamic_cast<Terror*>(
                findBuff(typeid(Terror))
            )) {

        if (ActorScheduler* s = scheduler()) {
            Char* source =
                dynamic_cast<Char*>(
                    s->findById(
                        terror->object
                    )
                );

            if (source) {
                return source;
            }
        }
    }

    return
        enemy_ &&
        enemy_->isAlive()
        ? enemy_
        : &dungeon_->hero();
}

bool Mob::actSleeping(bool enemyInFOV) {
    if (enemyInFOV && enemy_) {
        const int wakeRoll =
            distance(*enemy_) +
            enemy_->stealth() +
            (enemy_->flying ? 2 : 0);

        if (Random::Int(wakeRoll) == 0) {
            enemySeen_ = true;
            notice();

            state_ = State::Hunting;
            target_ = enemy_->pos;

            spend(TimeToWakeUp);
            next();
            return true;
        }
    }

    enemySeen_ = false;

    spend(TICK);
    next();

    return true;
}

bool Mob::actWandering(
    bool enemyInFOV,
    bool justAlerted
) {
    if (enemyInFOV &&
        enemy_ &&
        (justAlerted ||
         Random::Int(
             distance(*enemy_) / 2 +
             enemy_->stealth()
         ) == 0)) {

        enemySeen_ = true;
        notice();

        state_ = State::Hunting;
        target_ = enemy_->pos;

    } else {
        enemySeen_ = false;

        if (target_ != -1 &&
            getCloser(target_)) {

            spend(TICK / speed());
            next();
            return true;
        }

        target_ = randomDestination();
        spend(TICK);
    }

    next();
    return true;
}

bool Mob::actHunting(bool enemyInFOV) {
    enemySeen_ = enemyInFOV;

    if (enemyInFOV &&
        enemy_ &&
        canAttack(*enemy_)) {

        return doAttack(*enemy_);
    }

    if (enemyInFOV && enemy_) {
        target_ = enemy_->pos;
    }

    if (target_ != -1 &&
        getCloser(target_)) {

        spend(TICK / speed());
        next();

        return true;
    }

    spend(TICK);

    state_ = State::Wandering;
    target_ = randomDestination();

    next();
    return true;
}

bool Mob::actFleeing(bool enemyInFOV) {
    enemySeen_ = enemyInFOV;

    if (enemyInFOV && enemy_) {
        target_ = enemy_->pos;
    }

    if (target_ != -1 &&
        getFurther(target_)) {

        spend(TICK / speed());
        next();

        return true;
    }

    spend(TICK);
    next();

    return true;
}

bool Mob::actPassive() {
    enemySeen_ = false;

    spend(TICK);
    next();

    return true;
}

bool Mob::canAttack(const Char& enemy) const {
    return
        Level::adjacent(pos, enemy.pos) &&
        !isCharmedBy(enemy);
}

bool Mob::doAttack(Char& enemy) {
    // Original Mob.doAttack() resolves through a sprite callback only when the
    // mob is visible. Vita rendering is not the authority for game timing, so
    // the callback is resolved synchronously here while preserving attackDelay.
    if (dungeon_) dungeon_->noteMobAttack(pos);
    attack(enemy);
    spend(attackDelay());
    next();

    // If the Hero died, stop the scheduler immediately instead of allowing
    // AI-only turns to loop forever after the player actor was removed.
    return enemy.isAlive();
}

int Mob::defenseSkill(const Char&) const {
    // Original Mob.defenseSkill(): surprised/unaware mobs have zero defense.
    return enemySeen_ && !paralysed
        ? defenseSkill_
        : 0;
}

int Mob::defenseProc(Char&, int damage) {
    // Original Assassin surprise attack: unaware enemies take up to one
    // additional damage roll. The visual Wound effect is renderer-only.
    if (!enemySeen_ && dungeon_ &&
        dungeon_->hero().subClass() == HeroSubClass::Assassin) {
        damage += Random::IntRange(1, std::max(1, damage));
    }
    return damage;
}

void Mob::damage(int amount, const void* source) {
    // Original Mob.damage(): damage can immediately recover Terror.
    Terror::recover(*this);

    if (state_ == State::Sleeping) {
        state_ = State::Wandering;
    }

    alerted_ = true;
    Char::damage(amount, source);
}

void Mob::destroy() {
    // Original Mob.destroy(): remove actor, then award EXP while Hero is alive.
    Char::destroy();

    if (dungeon_ && dungeon_->hero().isAlive()) {
        const int reward = exp();
        if (reward > 0) {
            dungeon_->hero().earnExp(reward);
        }
        if (hostile_) {
            dungeon_->recordKill(isBoss());
            if (dynamic_cast<const Albino*>(this)) {
                Progression::instance().noteRare(0);
            } else if (const Thief* thief = dynamic_cast<const Thief*>(this); thief && thief->isBandit()) {
                Progression::instance().noteRare(1);
            } else if (const ZoneMob* zone = dynamic_cast<const ZoneMob*>(this)) {
                switch (zone->kind()) {
                    case ZoneMobKind::Shielded: Progression::instance().noteRare(2); break;
                    case ZoneMobKind::Senior:   Progression::instance().noteRare(3); break;
                    case ZoneMobKind::Acidic:   Progression::instance().noteRare(4); break;
                    default: break;
                }
            }
            dungeon_->postMessage("Killed "+name+(reward>0?" (+"+numberToString(reward)+" XP)":""));
        }
    }
}

int Mob::exp() const {
    if (!dungeon_) return 0;
    return dungeon_->hero().level() <= maxLvl_ ? exp_ : 0;
}

void Mob::die(const void* cause) {
    if(dungeon_&&hostile_)dungeon_->processQuestKill(*this,cause);
    // Luck raises all monster-drop chances by 20% per rank, capped so loot
    // remains valuable and the floor cannot be flooded with items.
    const float luckMultiplier=dungeon_?1.0f+0.20f*static_cast<float>(dungeon_->hero().luckUpgrades()):1.0f;
    const float dropChance=std::min(0.90f,lootChance()*luckMultiplier);
    if(dungeon_&&dropChance>0.0f&&Random::Float()<dropChance){
        std::unique_ptr<Item>loot=createLoot();if(loot)dungeon_->level().drop(std::move(loot),pos);
    }
    Char::die(cause);
    // Rat has no loot table entry, so no fake item is generated here.
}

std::unique_ptr<Item>Mob::createLoot()const{return nullptr;}

bool Mob::getCloser(int target) {
    if (rooted ||
        !dungeon_) {
        return false;
    }

    const int step =
        dungeon_->findPath(
            *this,
            pos,
            target,
            dungeon_->level().fieldOfView()
        );

    if (step == -1) {
        return false;
    }

    const int oldPos=pos;
    const bool moved =
        moveTo(
            step,
            dungeon_->level()
        );

    if(moved)onMoved(oldPos);
    if (moved &&
        !flying) {
        dungeon_
            ->level()
            .mobPress(*this);
    }

    return moved;
}

bool Mob::getFurther(int target) {
    if (!dungeon_) {
        return false;
    }

    const int step =
        dungeon_->flee(
            *this,
            pos,
            target,
            dungeon_->level().fieldOfView()
        );

    if (step == -1) {
        return false;
    }

    const int oldPos=pos;
    const bool moved =
        moveTo(
            step,
            dungeon_->level()
        );

    if(moved)onMoved(oldPos);
    if (moved &&
        !flying) {
        dungeon_
            ->level()
            .mobPress(*this);
    }

    return moved;
}

int Mob::randomDestination() const {
    if (!dungeon_) {
        return -1;
    }

    return dungeon_->level().randomDestination();
}

void Mob::beckon(int cell) {
    notice();

    if (state_ != State::Hunting) {
        state_ = State::Wandering;
    }

    target_ = cell;
}

void Mob::notice() {
    // Alert icon is presentation-only. AI transition is already handled.
}


void Mob::onBuffAdded(Buff& buff) {
    if (dynamic_cast<Amok*>(&buff)) {
        state_ = State::Hunting;
    } else if (dynamic_cast<Terror*>(&buff)) {
        state_ = State::Fleeing;
    } else if (dynamic_cast<Sleep*>(&buff)) {
        state_ = State::Sleeping;
        postpone(Sleep::SWS);
    }
}

void Mob::onBuffRemoved(Buff& buff) {
    if (dynamic_cast<Terror*>(&buff)) {
        state_ = State::Hunting;
    }
}

const char* Mob::description() const {
    return "Real description is coming soon!";
}

const char* mobStateName(Mob::State state) {
    switch (state) {
        case Mob::State::Sleeping:
            return "SLEEPING";
        case Mob::State::Wandering:
            return "WANDERING";
        case Mob::State::Hunting:
            return "HUNTING";
        case Mob::State::Fleeing:
            return "FLEEING";
        case Mob::State::Passive:
            return "PASSIVE";
    }

    return "UNKNOWN";
}

} // namespace pdv
