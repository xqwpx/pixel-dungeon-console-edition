#pragma once
#include <deque>
#include <memory>

namespace pdv {

class Item;

class Heap {
public:
    enum class Type {
        Heap,
        ForSale,
        Chest,
        LockedChest,
        CrystalChest,
        Tomb,
        Skeleton,
        Mimic,
        Hidden
    };

    explicit Heap(int pos = 0);

    Type type = Type::Heap;
    int pos = 0;

    int size() const { return static_cast<int>(items_.size()); }
    bool empty() const { return items_.empty(); }

    Item* peek() const;
    std::unique_ptr<Item> pickUp();
    void drop(std::unique_ptr<Item> item);
    void burn();
    void freeze();
    void replaceTop(std::unique_ptr<Item> item);
    const std::deque<std::unique_ptr<Item>>& items() const { return items_; }

private:
    std::deque<std::unique_ptr<Item>> items_;
};

} // namespace pdv
