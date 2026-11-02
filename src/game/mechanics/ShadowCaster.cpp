#include "game/mechanics/ShadowCaster.h"
#include "game/levels/Level.h"
#include <algorithm>
#include <array>
#include <cmath>

namespace pdv {
constexpr int ShadowCaster::MaxDistance;
namespace {

struct Obstacles {
    static constexpr int Size = (ShadowCaster::MaxDistance + 1) * (ShadowCaster::MaxDistance + 1) / 2;
    std::array<float, Size> a1{};
    std::array<float, Size> a2{};
    int length = 0;
    int limit = 0;

    void reset() { length = 0; limit = 0; }
    void add(float o1, float o2) {
        if (length > limit && o1 <= a2[static_cast<std::size_t>(length - 1)]) {
            a2[static_cast<std::size_t>(length - 1)] = o2;
        } else if (length < Size) {
            a1[static_cast<std::size_t>(length)] = o1;
            a2[static_cast<std::size_t>(length)] = o2;
            ++length;
        }
    }
    bool isBlocked(float a) const {
        for (int i = 0; i < limit; ++i) {
            if (a >= a1[static_cast<std::size_t>(i)] && a <= a2[static_cast<std::size_t>(i)]) return true;
        }
        return false;
    }
    void nextRow() { limit = length; }
};

std::array<std::array<int, ShadowCaster::MaxDistance + 1>, ShadowCaster::MaxDistance + 1> makeRounding() {
    std::array<std::array<int, ShadowCaster::MaxDistance + 1>, ShadowCaster::MaxDistance + 1> r{};
    for (int i = 1; i <= ShadowCaster::MaxDistance; ++i) {
        for (int j = 1; j <= i; ++j) {
            const double v = i * std::cos(std::asin(static_cast<double>(j) / (i + 0.5)));
            r[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] =
                static_cast<int>(std::min<double>(j, std::floor(v + 0.5)));
        }
    }
    return r;
}

const auto Rounding = makeRounding();

void scanSector(int cx, int cy, int m1, int m2, int m3, int m4,
                int distance,
                const std::array<bool, Level::Length>& losBlocking,
                std::array<bool, Level::Length>& fov) {
    Obstacles obs;
    obs.reset();
    const auto& limits = Rounding[static_cast<std::size_t>(distance)];

    for (int p = 1; p <= distance; ++p) {
        const float dq2 = 0.5f / static_cast<float>(p);
        const int pp = limits[static_cast<std::size_t>(p)];
        for (int q = 0; q <= pp; ++q) {
            const int x = cx + q * m1 + p * m3;
            const int y = cy + p * m2 + q * m4;
            if (y >= 0 && y < Level::Height && x >= 0 && x < Level::Width) {
                const float a0 = static_cast<float>(q) / static_cast<float>(p);
                const float a1 = a0 - dq2;
                const float a2 = a0 + dq2;
                const int pos = y * Level::Width + x;
                if (!(obs.isBlocked(a0) && obs.isBlocked(a1) && obs.isBlocked(a2))) {
                    fov[static_cast<std::size_t>(pos)] = true;
                }
                if (losBlocking[static_cast<std::size_t>(pos)]) obs.add(a1, a2);
            }
        }
        obs.nextRow();
    }
}

} // namespace

void ShadowCaster::castShadow(int x, int y,
                              const std::array<bool,1024>& losBlocking,
                              std::array<bool,1024>& fieldOfView,
                              int distance) {
    distance = std::max(1, std::min(MaxDistance, distance));
    fieldOfView.fill(false);
    if (x < 0 || y < 0 || x >= Level::Width || y >= Level::Height) return;
    fieldOfView[static_cast<std::size_t>(y * Level::Width + x)] = true;

    scanSector(x,y,+1,+1,0,0,distance,losBlocking,fieldOfView);
    scanSector(x,y,-1,+1,0,0,distance,losBlocking,fieldOfView);
    scanSector(x,y,+1,-1,0,0,distance,losBlocking,fieldOfView);
    scanSector(x,y,-1,-1,0,0,distance,losBlocking,fieldOfView);
    scanSector(x,y,0,0,+1,+1,distance,losBlocking,fieldOfView);
    scanSector(x,y,0,0,-1,+1,distance,losBlocking,fieldOfView);
    scanSector(x,y,0,0,+1,-1,distance,losBlocking,fieldOfView);
    scanSector(x,y,0,0,-1,-1,distance,losBlocking,fieldOfView);
}

} // namespace pdv
