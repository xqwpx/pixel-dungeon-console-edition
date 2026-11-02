#pragma once

#include <memory>

namespace pdv {

class Mob;

class Bestiary {
public:
    static std::unique_ptr<Mob> mob(int depth);
    static std::unique_ptr<Mob> mutableMob(int depth);
};

} // namespace pdv
