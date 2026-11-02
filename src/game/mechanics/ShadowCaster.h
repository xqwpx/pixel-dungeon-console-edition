#pragma once
#include <array>

namespace pdv {
class Level;

class ShadowCaster {
public:
    static constexpr int MaxDistance = 8;
    static void castShadow(int x, int y,
                           const std::array<bool,1024>& losBlocking,
                           std::array<bool,1024>& fieldOfView,
                           int distance);
};
} // namespace pdv
