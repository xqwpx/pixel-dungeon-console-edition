#pragma once
#include "game/items/Bag.h"
namespace pdv {
class SeedPouch : public Bag {
public:
    SeedPouch();
    bool grab(const Item& item) const override;
    int price() const override { return 50; }
    std::unique_ptr<Item> clone() const override;
};
}
