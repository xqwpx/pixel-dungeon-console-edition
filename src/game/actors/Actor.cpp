#include "game/actors/Actor.h"

#include "game/actors/Char.h"
#include "game/actors/buffs/Buff.h"
#include "game/levels/Level.h"

#include <algorithm>
#include <limits>

namespace pdv {

int Actor::id() {
    if (id_ > 0) {
        return id_;
    }

    int maximum = 0;

    if (scheduler_) {
        for (Actor* actor : scheduler_->all()) {
            if (actor && actor->id_ > maximum) {
                maximum = actor->id_;
            }
        }
    }

    id_ = maximum + 1;
    return id_;
}

void Actor::spend(float value) {
    time_ += value;
}

void Actor::postpone(float value) {
    const float base = scheduler_ ? scheduler_->now() : 0.0f;

    if (time_ < base + value) {
        time_ = base + value;
    }
}

float Actor::cooldown() const {
    return time_ - (scheduler_ ? scheduler_->now() : 0.0f);
}

void Actor::deactivate() {
    time_ = std::numeric_limits<float>::max();
}

void Actor::next() {
    if (scheduler_) {
        scheduler_->release(this);
    }
}

void ActorScheduler::clear() {
    for (Actor* actor : all_) {
        if (actor) {
            actor->scheduler_ = nullptr;
        }
    }

    all_.clear();
    chars_.assign(Level::Length, nullptr);

    current_ = nullptr;
    now_ = 0.0f;
}

void ActorScheduler::fixTime() {
    float minTime = std::numeric_limits<float>::max();

    for (Actor* actor : all_) {
        if (actor && actor->time_ < minTime) {
            minTime = actor->time_;
        }
    }

    if (minTime == std::numeric_limits<float>::max()) {
        now_ = 0.0f;
        return;
    }

    for (Actor* actor : all_) {
        if (actor) {
            actor->time_ -= minTime;
        }
    }

    now_ = 0.0f;
}

void ActorScheduler::add(Actor* actor) {
    addAt(actor, now_);
}

void ActorScheduler::addDelayed(Actor* actor, float delay) {
    addAt(actor, now_ + delay);
}

void ActorScheduler::addAt(Actor* actor, float time) {
    if (!actor ||
        std::find(all_.begin(), all_.end(), actor) != all_.end()) {
        return;
    }

    if (chars_.size() != Level::Length) {
        chars_.assign(Level::Length, nullptr);
    }

    all_.push_back(actor);

    actor->scheduler_ = this;
    actor->time_ += time;
    actor->onAdd();

    if (Char* ch = dynamic_cast<Char*>(actor)) {
        occupyCell(ch);

        // Original Actor.add(Char) re-registers all buffs owned by that Char.
        for (Buff* buff : ch->activeBuffs()) {
            if (buff) {
                addAt(buff, 0.0f);
            }
        }
    }
}

void ActorScheduler::remove(Actor* actor) {
    if (!actor) {
        return;
    }

    if (Char* ch = dynamic_cast<Char*>(actor)) {
        freeCell(ch->pos);
    }

    const auto it = std::find(all_.begin(), all_.end(), actor);

    if (it != all_.end()) {
        all_.erase(it);
        actor->onRemove();
        actor->scheduler_ = nullptr;
    }

    if (current_ == actor) {
        current_ = nullptr;
    }
}

void ActorScheduler::release(Actor* actor) {
    if (current_ == actor) {
        current_ = nullptr;
    }
}

void ActorScheduler::rebuildCharMap() {
    chars_.assign(Level::Length, nullptr);

    for (Actor* actor : all_) {
        Char* ch = dynamic_cast<Char*>(actor);

        if (!ch ||
            ch->pos < 0 ||
            ch->pos >= Level::Length ||
            !ch->isAlive()) {
            continue;
        }

        chars_[static_cast<std::size_t>(ch->pos)] = ch;
    }
}

void ActorScheduler::process(Actor* stopIfMissing) {
    if (current_ != nullptr) {
        return;
    }

    bool doNext = false;
    // No normal turn needs more than a few actor actions before the hero is
    // scheduled again.  This guard prevents a malformed actor transition from
    // locking the entire game loop (notably while a boss is being removed).
    const std::size_t maxActions = std::max<std::size_t>(256, all_.size() * 32);
    std::size_t actions = 0;

    do {
        if (stopIfMissing &&
            std::find(all_.begin(), all_.end(), stopIfMissing) == all_.end()) {
            current_ = nullptr;
            return;
        }

        now_ = std::numeric_limits<float>::max();
        current_ = nullptr;

        rebuildCharMap();

        for (Actor* actor : all_) {
            if (actor && actor->time_ < now_) {
                now_ = actor->time_;
                current_ = actor;
            }
        }

        if (current_) {
            doNext = current_->act();

            if (++actions >= maxActions) {
                current_ = nullptr;
                return;
            }

            if (stopIfMissing &&
                std::find(all_.begin(), all_.end(), stopIfMissing) == all_.end()) {
                doNext = false;
                current_ = nullptr;
            }
        } else {
            doNext = false;
        }

    } while (doNext);
}

void ActorScheduler::occupyCell(Char* ch) {
    if (!ch ||
        ch->pos < 0 ||
        ch->pos >= Level::Length) {
        return;
    }

    if (chars_.size() != Level::Length) {
        chars_.assign(Level::Length, nullptr);
    }

    chars_[static_cast<std::size_t>(ch->pos)] = ch;
}

void ActorScheduler::freeCell(int pos) {
    if (pos < 0 ||
        pos >= static_cast<int>(chars_.size())) {
        return;
    }

    chars_[static_cast<std::size_t>(pos)] = nullptr;
}

Char* ActorScheduler::findChar(int pos) const {
    if (pos < 0 ||
        pos >= static_cast<int>(chars_.size())) {
        return nullptr;
    }

    return chars_[static_cast<std::size_t>(pos)];
}

Actor* ActorScheduler::findById(int id) const {
    if (id <= 0) {
        return nullptr;
    }

    for (Actor* actor : all_) {
        if (actor && actor->id_ == id) {
            return actor;
        }
    }

    return nullptr;
}

} // namespace pdv
