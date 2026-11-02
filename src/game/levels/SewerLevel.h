#pragma once
#include "game/levels/RegularLevel.h"
namespace pdv {
class SewerLevel:public RegularLevel{
protected:
 int water() const override;
 int grass() const override;
 void decorate() override;
};
}
