#include "game/items/KindOfWeapon.h"
#include "game/actors/Hero.h"
#include "game/utils/Random.h"

namespace pdv {

std::vector<ItemAction> KindOfWeapon::actions(const Hero& hero) const {
    const bool equipped=hero.belongings().weapon()==this;
    return {equipped?ItemAction::Unequip:ItemAction::Equip,ItemAction::Drop,ItemAction::Throw,ItemAction::QuickSlot};
}

int KindOfWeapon::damageRoll(const Hero&) const {
    return Random::NormalIntRange(min(), max());
}

} // namespace pdv
