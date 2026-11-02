#include "game/actors/Belongings.h"
#include "game/items/KindOfWeapon.h"
#include "game/items/armor/Armor.h"
#include "game/items/keys/Key.h"
#include "game/items/rings/Ring.h"
#include "game/items/wands/Wand.h"

namespace pdv {

Belongings::Belongings(Hero* owner)
    : owner_(owner) {

    backpack_.owner(owner);
}

Belongings::~Belongings() = default;

void Belongings::reset(Hero* owner) {
    owner_ = owner;
    backpack_.clear();
    backpack_.owner(owner);
    weapon_.reset();
    armor_.reset();
    ring1_.reset();
    ring2_.reset();
}

void Belongings::equipWeapon(std::unique_ptr<KindOfWeapon> weapon) {
    weapon_ = std::move(weapon);
}

void Belongings::equipArmor(std::unique_ptr<Armor> armor) {
    armor_ = std::move(armor);
}

bool Belongings::equipRing(std::unique_ptr<Ring> ring) {
    if (!ring) return false;
    if (!ring1_) { ring1_=std::move(ring); return true; }
    if (!ring2_) { ring2_=std::move(ring); return true; }
    return false;
}

std::unique_ptr<KindOfWeapon> Belongings::takeWeapon() {
    return std::move(weapon_);
}

std::unique_ptr<Armor> Belongings::takeArmor() {
    return std::move(armor_);
}

std::unique_ptr<Ring> Belongings::takeRing(Ring* ring) {
    if (ring1_.get()==ring) return std::move(ring1_);
    if (ring2_.get()==ring) return std::move(ring2_);
    return nullptr;
}



namespace {

void collectBagItems(
    const Bag& bag,
    std::vector<Item*>& out
) {
    for (const auto& item :
         bag.items()) {

        if (!item) {
            continue;
        }

        out.push_back(
            item.get()
        );

        const Bag* nested =
            dynamic_cast<const Bag*>(
                item.get()
            );

        if (nested) {
            collectBagItems(
                *nested,
                out
            );
        }
    }
}

} // namespace

std::vector<Item*>
Belongings::allItems() const {
    std::vector<Item*> result;

    if (weapon_) {
        result.push_back(
            weapon_.get()
        );
    }

    if (armor_) {
        result.push_back(
            armor_.get()
        );
    }

    if (ring1_) {
        result.push_back(
            ring1_.get()
        );
    }

    if (ring2_) {
        result.push_back(
            ring2_.get()
        );
    }

    collectBagItems(
        backpack_,
        result
    );

    return result;
}

int Belongings::charge(bool full) {
    int count = 0;

    for (Item* item :
         allItems()) {

        Wand* wand =
            dynamic_cast<Wand*>(
                item
            );

        if (wand &&
            wand->recharge(full)) {
            ++count;
        }
    }

    return count;
}

int Belongings::uncurseEquipped() {
    int count = 0;

    for (Item* item : {static_cast<Item*>(weapon_.get()), static_cast<Item*>(armor_.get()), static_cast<Item*>(ring1_.get()), static_cast<Item*>(ring2_.get())}) {
        if (item && item->cursed()) {
            item->setCursed(false);
            ++count;
        }
    }

    return count;
}

int Belongings::uncurseAll() {
    int count = 0;

    for (Item* item :
         allItems()) {

        if (item &&
            item->cursed()) {

            item->setCursed(false);
            ++count;
        }
    }

    return count;
}

void Belongings::observeEquipped() {
    for (Item* item : {static_cast<Item*>(weapon_.get()), static_cast<Item*>(armor_.get()), static_cast<Item*>(ring1_.get()), static_cast<Item*>(ring2_.get())}) {
        if (item) item->identify();
    }

    std::vector<Item*> backpackItems;
    collectBagItems(backpack_, backpackItems);
    for (Item* item : backpackItems) {
        if (item) item->setCursedKnown(true);
    }
}

void Belongings::resurrect(int depth) {
    auto& items = backpack_.itemsMutable();
    items.erase(std::remove_if(items.begin(), items.end(), [depth](const std::unique_ptr<Item>& item) {
        if (!item) return true;
        if (const Key* key = dynamic_cast<const Key*>(item.get())) return key->depth() == depth;
        return !item->unique();
    }), items.end());

    if (weapon_) weapon_->setCursed(false);
    if (armor_) armor_->setCursed(false);
    if (ring1_) ring1_->setCursed(false);
    if (ring2_) ring2_->setCursed(false);
}
} // namespace pdv
