#include "game/actors/buffs/Shadows.h"

#include "game/actors/mobs/Mob.h"
#include "game/levels/Level.h"

namespace pdv {

bool Shadows::attachTo(Char& target) {
    return Invisibility::attachTo(target);
}

void Shadows::detach() {
    Invisibility::detach();
}

bool Shadows::act() {
    Char* ch = target();

    if (!ch || !ch->isAlive()) {
        detach();
        return true;
    }

    spend(TICK * 2.0f);

    left_ -= 1.0f;

    bool visibleEnemy = false;

    ActorScheduler* s =
        attachedScheduler();

    Level* level =
        ch->levelContext();

    if (s && level) {
        const auto& fov =
            level->fieldOfView();

        for (Actor* actor : s->all()) {
            Mob* mob =
                dynamic_cast<Mob*>(actor);

            if (!mob ||
                !mob->isAlive() ||
                !mob->hostile() ||
                !level->inBoundsCell(mob->pos)) {
                continue;
            }

            if (fov[
                    static_cast<std::size_t>(
                        mob->pos
                    )
                ]) {
                visibleEnemy = true;
                break;
            }
        }
    }

    if (left_ <= 0.0f ||
        visibleEnemy) {
        detach();
    } else {
        next();
    }

    return true;
}

} // namespace pdv
