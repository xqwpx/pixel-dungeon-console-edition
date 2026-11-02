#include "game/actors/mobs/npcs/MirrorImage.h"

#include "game/Dungeon.h"
#include "game/actors/Hero.h"
#include "game/levels/Level.h"
#include "game/utils/Random.h"

#include <vector>

namespace pdv {

MirrorImage::MirrorImage() {
    name = "mirror image";

    HP = HT = 1;
    exp_ = 0;

    hostile_ = false;
    state_ = State::Hunting;
}

void MirrorImage::duplicate(
    const Hero& hero
) {
    attack_ =
        hero.attackSkill(hero);

    damage_ =
        hero.damageRoll();
}

int MirrorImage::attackSkill(
    const Char&
) const {
    return attack_;
}

int MirrorImage::damageRoll() const {
    return damage_;
}

int MirrorImage::attackProc(
    Char& enemy,
    int damage
) {
    const int result =
        Mob::attackProc(
            enemy,
            damage
        );

    destroy();
    return result;
}

Char* MirrorImage::chooseEnemy() {
    if (!dungeon_) {
        return nullptr;
    }

    if (enemy_ &&
        enemy_->isAlive() &&
        enemy_ !=
            &dungeon_->hero()) {
        return enemy_;
    }

    std::vector<Mob*> enemies;

    for (auto& mob :
         dungeon_
             ->level()
             .mobsMutable()) {

        if (!mob ||
            mob.get() == this ||
            !mob->isAlive() ||
            !mob->hostile()) {
            continue;
        }

        if (dungeon_
                ->level()
                .fieldOfView()[
                    static_cast<std::size_t>(
                        mob->pos
                    )
                ]) {

            enemies.push_back(
                mob.get()
            );
        }
    }

    return Random::element(enemies);
}

} // namespace pdv
