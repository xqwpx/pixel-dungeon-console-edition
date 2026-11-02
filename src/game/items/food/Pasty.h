#pragma once
#include "game/items/food/Food.h"
namespace pdv {
class Pasty : public Food {
public:
    Pasty();
    int price() const override { return 20*quantity(); }
    std::unique_ptr<Item> clone() const override;
};
}
