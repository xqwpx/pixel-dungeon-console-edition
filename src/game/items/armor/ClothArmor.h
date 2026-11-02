#pragma once
#include "game/items/armor/Armor.h"

namespace pdv {

class ClothArmor : public Armor {
public:
    ClothArmor();
    std::unique_ptr<Item> clone() const override;
};

} // namespace pdv
