#include "game/ui/InventoryModel.h"

#include "game/actors/Hero.h"
#include "game/items/Bag.h"
#include "game/items/Item.h"
#include "game/items/armor/Armor.h"
#include "game/items/rings/Ring.h"
#include "game/items/weapon/Weapon.h"

#include <algorithm>
#include <cstdlib>
#include <limits>

namespace pdv {

void InventoryModel::bind(
    Hero* hero
) {
    hero_ = hero;
    rebuild();
}

void InventoryModel::open(
    InventoryMode mode,
    const std::string& title
) {
    open_ = true;
    mode_ = mode;
    title_ = title;

    rebuild();
}

void InventoryModel::close() {
    closeContext();
    open_ = false;
    selected_ = -1;
}

bool InventoryModel::openContext() {
    Item* item=selectedItem();
    if (!item || !hero_) return false;
    contextActions_=item->actions(*hero_);
    contextOpen_=!contextActions_.empty(); contextSelected_=contextOpen_?0:-1;
    return contextOpen_;
}
void InventoryModel::closeContext(){ contextOpen_=false;contextActions_.clear();contextSelected_=-1; }
ItemAction InventoryModel::selectedAction() const { return contextSelected_>=0&&contextSelected_<static_cast<int>(contextActions_.size())?contextActions_[static_cast<std::size_t>(contextSelected_)]:ItemAction::Use; }
void InventoryModel::moveContextSelection(int delta){ if(contextActions_.empty())return; contextSelected_=(contextSelected_+delta+static_cast<int>(contextActions_.size()))%static_cast<int>(contextActions_.size()); }

void InventoryModel::setMode(
    InventoryMode mode,
    const std::string& title
) {
    mode_ = mode;
    title_ = title;
    rebuild();
}

const std::string&
InventoryModel::tabName(
    int index
) const {
    static const std::string empty;

    if (index < 0 ||
        index >=
            static_cast<int>(
                tabs_.size()
            )) {
        return empty;
    }

    return tabs_[
        static_cast<std::size_t>(
            index
        )
    ].name;
}

bool InventoryModel::accepts(
    const Item& item
) const {
    switch (mode_) {
        case InventoryMode::All:
            return true;

        case InventoryMode::Unidentified:
            return
                !item.isIdentified();

        case InventoryMode::Upgradable:
            return
                item.isUpgradable();

        case InventoryMode::Enchantable:
            return
                dynamic_cast<
                    const Weapon*
                >(&item) != nullptr ||
                dynamic_cast<
                    const Armor*
                >(&item) != nullptr;
    }

    return false;
}

void InventoryModel::rebuild() {
    tabs_.clear();
    slots_.clear();

    if (!hero_) {
        selected_ = -1;
        return;
    }

    Bag& backpack =
        hero_
            ->belongings()
            .backpack();

    // The Vita layout has one unified backpack: contents of special bags are
    // shown directly instead of requiring L1/R1 tab switching.
    tabs_.push_back({&backpack, "B"});
    activeTab_ = 0;

    // Original WndBag always displays equipped weapon/armor/rings first.
    Item* equipped[4] = {
        hero_->belongings().weapon(),
        hero_->belongings().armor(),
        hero_->belongings().ring1(),
        hero_->belongings().ring2()
    };

    int col = 0;
    int row = 0;
    const auto addSlot = [&](Item* item, bool equipped) {
        slots_.push_back({item, col, row, accepts(*item), equipped});
        if (++col >= Columns) { col = 0; ++row; }
    };

    for (int i = 0; i < 4; ++i) {
        if (!equipped[i]) {
            continue;
        }

        addSlot(equipped[i], true);
    }

    const auto addItem = [&](Item* item) {
        if (!item) return;
        for (Item* equippedItem : equipped) {
            if (item == equippedItem) return;
        }
        addSlot(item, false);
    };
    const auto addBagContents = [&](const auto& self,const Bag& bag) -> void {
        for (const auto& owned : bag.items()) {
            if (!owned) continue;
            if (const Bag* nested = dynamic_cast<const Bag*>(owned.get())) {
                self(self,*nested);
            } else {
                addItem(owned.get());
            }
        }
    };
    addBagContents(addBagContents,backpack);

    if (selected_ < 0 ||
        selected_ >=
            static_cast<int>(
                slots_.size()
            ) ||
        !slots_[
            static_cast<std::size_t>(
                selected_
            )
        ].enabled) {

        selected_ = -1;

        for (int i = 0;
             i <
                static_cast<int>(
                    slots_.size()
                );
             ++i) {

            if (slots_[
                    static_cast<std::size_t>(
                        i
                    )
                ].enabled) {

                selected_ = i;
                break;
            }
        }
    }
}

Item* InventoryModel::selectedItem() const {
    if (selected_ < 0 ||
        selected_ >=
            static_cast<int>(
                slots_.size()
            )) {
        return nullptr;
    }

    const InventorySlot& slot =
        slots_[
            static_cast<std::size_t>(
                selected_
            )
        ];

    return slot.enabled
        ? slot.item
        : nullptr;
}

void InventoryModel::selectIndex(
    int index
) {
    if (index < 0 ||
        index >=
            static_cast<int>(
                slots_.size()
            )) {
        return;
    }

    if (!slots_[
            static_cast<std::size_t>(
                index
            )
        ].enabled) {
        return;
    }

    selected_ = index;
}

int InventoryModel::findNearest(
    int col,
    int row,
    int dx,
    int dy
) const {
    int best = -1;
    int bestScore =
        std::numeric_limits<int>::max();

    for (int i = 0;
         i <
            static_cast<int>(
                slots_.size()
            );
         ++i) {

        const InventorySlot& slot =
            slots_[
                static_cast<std::size_t>(
                    i
                )
            ];

        if (!slot.enabled) {
            continue;
        }

        const int rx =
            slot.col - col;

        const int ry =
            slot.row - row;

        if (dx < 0 && rx >= 0) continue;
        if (dx > 0 && rx <= 0) continue;
        if (dy < 0 && ry >= 0) continue;
        if (dy > 0 && ry <= 0) continue;

        const int primary =
            dx != 0
            ? std::abs(rx)
            : std::abs(ry);

        const int secondary =
            dx != 0
            ? std::abs(ry)
            : std::abs(rx);

        const int score =
            primary * 100 +
            secondary;

        if (score < bestScore) {
            bestScore = score;
            best = i;
        }
    }

    return best;
}

void InventoryModel::moveSelection(
    int dx,
    int dy
) {
    if (slots_.empty()) {
        selected_ = -1;
        return;
    }

    if (selected_ < 0) {
        rebuild();
        return;
    }

    const InventorySlot& current =
        slots_[
            static_cast<std::size_t>(
                selected_
            )
        ];

    const int next =
        findNearest(
            current.col,
            current.row,
            dx,
            dy
        );

    if (next >= 0) {
        selected_ = next;
    }
}

void InventoryModel::nextTab() {
    if (tabs_.empty()) {
        return;
    }

    activeTab_ =
        (
            activeTab_ + 1
        ) %
        static_cast<int>(
            tabs_.size()
        );

    selected_ = -1;
    rebuild();
}

void InventoryModel::previousTab() {
    if (tabs_.empty()) {
        return;
    }

    activeTab_ =
        (
            activeTab_ -
            1 +
            static_cast<int>(
                tabs_.size()
            )
        ) %
        static_cast<int>(
            tabs_.size()
        );

    selected_ = -1;
    rebuild();
}

} // namespace pdv
