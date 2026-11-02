#include "game/items/Bag.h"
#include <algorithm>

namespace pdv {
Bag::Bag(int size, const std::string& name) : size_(size) { name_ = name; }

void Bag::owner(Hero* hero) {
    owner_ = hero;
    for (auto& item : items_) {
        if (Bag* bag = dynamic_cast<Bag*>(item.get())) bag->owner(hero);
    }
}


bool Bag::canCollect(const Item& item) const {
    for (const auto& existing : items_) {
        const Bag* bag = dynamic_cast<const Bag*>(existing.get());
        if (bag && bag->grab(item)) return bag->canCollect(item);
    }
    if (item.stackable()) {
        for (const auto& existing:items_) if (existing && existing->sameKind(item)) return true;
    }
    return static_cast<int>(items_.size()) < size_;
}

bool Bag::collect(std::unique_ptr<Item> item) {
    if (!item) return false;

    // Original Item.collect(): existing nested bags get first chance to grab it.
    for (auto& existing : items_) {
        Bag* bag = dynamic_cast<Bag*>(existing.get());
        if (bag && bag->grab(*item)) return bag->collect(std::move(item));
    }

    if (item->stackable()) {
        for (auto& existing : items_) {
            if (existing && existing->sameKind(*item)) {
                existing->quantity(existing->quantity() + item->quantity());
                return true;
            }
        }
    }

    if (static_cast<int>(items_.size()) >= size_) return false;

    Bag* addedBag = dynamic_cast<Bag*>(item.get());
    items_.push_back(std::move(item));

    if (addedBag) {
        addedBag->owner(owner_);
        addedBag->adoptMatchingItems(*this);
    }
    return true;
}

void Bag::adoptMatchingItems(Bag& container) {
    // Mirrors Bag.collect(): when a specialized bag is added, it moves matching
    // items from the parent container into itself.
    for (auto it = container.items_.begin(); it != container.items_.end();) {
        if (it->get() == this) { ++it; continue; }
        if (*it && grab(**it)) {
            auto moved = std::move(*it);
            it = container.items_.erase(it);
            collect(std::move(moved));
        } else {
            ++it;
        }
    }
}

std::unique_ptr<Item> Bag::removeAll(Item* item) {
    if (!item) return nullptr;
    for (auto it=items_.begin(); it!=items_.end(); ++it) {
        if (it->get() == item) {
            auto out=std::move(*it); items_.erase(it); return out;
        }
        if (Bag* bag = dynamic_cast<Bag*>(it->get())) {
            auto nested=bag->removeAll(item); if (nested) return nested;
        }
    }
    return nullptr;
}

std::unique_ptr<Item> Bag::detach(Item* item) {
    if (!item) return nullptr;
    for (auto& owned : items_) {
        if (owned.get() == item) {
            if (owned->quantity() > 1) {
                owned->quantity(owned->quantity()-1);
                auto one=owned->clone(); one->quantity(1); return one;
            }
            return removeAll(item);
        }
        if (Bag* bag=dynamic_cast<Bag*>(owned.get())) {
            auto nested=bag->detach(item); if (nested) return nested;
        }
    }
    return nullptr;
}

bool Bag::contains(const Item* item) const {
    for (const auto& owned:items_) {
        if (owned.get()==item) return true;
        if (const Bag* bag=dynamic_cast<const Bag*>(owned.get())) if (bag->contains(item)) return true;
    }
    return false;
}

void Bag::clear(){ items_.clear(); }

std::unique_ptr<Item> Bag::clone() const {
    return std::unique_ptr<Item>(new Bag(size_, name_));
}
} // namespace pdv
