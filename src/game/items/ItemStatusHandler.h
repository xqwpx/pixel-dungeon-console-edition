#pragma once
#include <array>
#include <cstdint>
#include <string>

namespace pdv {

class ItemStatusHandler {
public:
    static constexpr int Capacity = 12;

    void init(
        const std::array<std::string, Capacity>& labels,
        const std::array<int, Capacity>& images
    );

    const std::string& label(int itemIndex) const;
    int image(int itemIndex) const;

    bool isKnown(int itemIndex) const;
    void know(int itemIndex);

    int knownCount() const;
    bool allKnown() const;
    uint16_t knownMask() const;
    void restoreKnownMask(uint16_t mask);

private:
    std::array<std::string, Capacity> labels_{};
    std::array<int, Capacity> images_{};
    std::array<bool, Capacity> known_{};
};

} // namespace pdv
