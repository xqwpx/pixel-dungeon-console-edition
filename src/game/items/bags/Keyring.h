#pragma once
#include "game/items/Bag.h"
namespace pdv {
class Keyring : public Bag {
public:
    Keyring();
    bool grab(const Item& item) const override;
    std::unique_ptr<Item> clone() const override;
};
} // namespace pdv
