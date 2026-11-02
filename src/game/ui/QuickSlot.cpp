#include "game/ui/QuickSlot.h"
#include "game/actors/Hero.h"
#include "game/items/weapon/missiles/Dart.h"
#include "game/items/weapon/missiles/Boomerang.h"
#include "game/items/wands/WandOfMagicMissile.h"
#include "game/items/wands/Wand.h"
#include "game/items/weapon/missiles/MissileWeapon.h"
#include <algorithm>
namespace pdv {
Item* QuickSlot::select(const Hero& hero) const {
    // An instance slot must never leave a dangling pointer after an item has
    // been used, sold or thrown.  The item can be nested in a bag, therefore
    // use Belongings::allItems rather than only inspecting the backpack root.
    const auto stillOwned = [&hero, this] {
        const auto items = hero.belongings().allItems();
        return instance_ && std::find(items.begin(), items.end(), instance_) != items.end();
    };
    switch(kind_){
        case QuickSlotKind::Dart: return hero.belongings().getItem<Dart>();
        case QuickSlotKind::WandOfMagicMissile:return stillOwned()?instance_:hero.belongings().getItem<WandOfMagicMissile>();
        case QuickSlotKind::Boomerang:return stillOwned()?instance_:hero.belongings().getItem<Boomerang>();
        case QuickSlotKind::Wand:return stillOwned()?instance_:hero.belongings().getItem<Wand>();
        case QuickSlotKind::Missile:return stillOwned()?instance_:hero.belongings().getItem<MissileWeapon>();
        case QuickSlotKind::Item:return stillOwned()?instance_:nullptr;
        default: return nullptr;
    }
}
} // namespace pdv
