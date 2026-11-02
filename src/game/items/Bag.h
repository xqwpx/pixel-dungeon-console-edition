#pragma once
#include <memory>
#include <vector>
#include "game/items/Item.h"

namespace pdv {
class Hero;

class Bag : public Item {
public:
    explicit Bag(int size = 1, const std::string& name = "bag");
    ~Bag() override = default;

    int size() const { return size_; }
    // The hero's Endurance upgrade expands only the main backpack.  Keep the
    // limit on the bag itself so every collection path uses the same capacity.
    void setSize(int size) { size_ = size < 1 ? 1 : size; }
    int count() const { return static_cast<int>(items_.size()); }
    Hero* owner() const { return owner_; }
    void owner(Hero* hero);

    const std::vector<std::unique_ptr<Item>>& items() const { return items_; }
    std::vector<std::unique_ptr<Item>>& itemsMutable() { return items_; }

    virtual bool grab(const Item& item) const { (void)item; return false; }

    bool canCollect(const Item& item) const;
    bool collect(std::unique_ptr<Item> item);
    std::unique_ptr<Item> detach(Item* item);
    std::unique_ptr<Item> removeAll(Item* item);
    bool contains(const Item* item) const;
    void clear();

    template<class T>
    T* findFirst() const {
        for (const auto& item : items_) {
            if (!item) continue;
            if (T* hit = dynamic_cast<T*>(item.get())) return hit;
            if (const Bag* bag = dynamic_cast<const Bag*>(item.get())) {
                if (T* nested = bag->template findFirst<T>()) return nested;
            }
        }
        return nullptr;
    }

    template<class T>
    T* findKey() const {
        for (const auto& item : items_) {
            if (!item) continue;
            if (T* hit = dynamic_cast<T*>(item.get())) {
                return hit;
            }
            if (const Bag* bag = dynamic_cast<const Bag*>(item.get())) {
                if (T* nested = bag->template findKey<T>()) return nested;
            }
        }
        return nullptr;
    }

    std::unique_ptr<Item> clone() const override;
    bool isUpgradable() const override { return false; }
    bool isIdentified() const override { return true; }

protected:
    void adoptMatchingItems(Bag& container);

    int size_ = 1;
    Hero* owner_ = nullptr;
    std::vector<std::unique_ptr<Item>> items_;
};
} // namespace pdv
