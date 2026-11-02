#pragma once

#include <vector>

namespace pdv {

class Char;
class ActorScheduler;

class Actor {
public:
    static constexpr float TICK = 1.0f;

    virtual ~Actor() = default;
    virtual bool act() = 0;

    float time() const { return time_; }
    float cooldown() const;
    void resetTime() { time_ = 0.0f; }

    int id();

    virtual void spend(float time);
    void postpone(float time);
    void deactivate();
    void next();

    ActorScheduler* attachedScheduler() const { return scheduler_; }

protected:
    virtual void onAdd() {}
    virtual void onRemove() {}

    ActorScheduler* scheduler() const { return scheduler_; }

private:
    friend class ActorScheduler;

    float time_ = 0.0f;
    int id_ = 0;
    ActorScheduler* scheduler_ = nullptr;
};

class ActorScheduler {
public:
    void clear();
    void fixTime();

    void add(Actor* actor);
    void addDelayed(Actor* actor, float delay);
    void remove(Actor* actor);

    void process(Actor* stopIfMissing = nullptr);

    void occupyCell(Char* ch);
    void freeCell(int pos);
    Char* findChar(int pos) const;

    Actor* findById(int id) const;

    float now() const { return now_; }
    Actor* current() const { return current_; }

    const std::vector<Actor*>& all() const { return all_; }

private:
    friend class Actor;

    void release(Actor* actor);
    void addAt(Actor* actor, float time);
    void rebuildCharMap();

    std::vector<Actor*> all_;
    std::vector<Char*> chars_;

    Actor* current_ = nullptr;
    float now_ = 0.0f;
};

} // namespace pdv
