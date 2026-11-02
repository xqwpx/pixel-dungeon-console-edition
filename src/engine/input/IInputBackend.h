#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace pdv {

// Compatibility values are deliberately kept at the engine edge while older
// scene bindings are migrated to GameButton names. They are not Vita headers.
constexpr uint32_t SCE_CTRL_SELECT = 0x00000001, SCE_CTRL_START = 0x00000008;
constexpr uint32_t SCE_CTRL_UP = 0x00000010, SCE_CTRL_RIGHT = 0x00000020;
constexpr uint32_t SCE_CTRL_DOWN = 0x00000040, SCE_CTRL_LEFT = 0x00000080;
constexpr uint32_t SCE_CTRL_TRIANGLE = 0x00001000, SCE_CTRL_CIRCLE = 0x00002000;
constexpr uint32_t SCE_CTRL_CROSS = 0x00004000, SCE_CTRL_SQUARE = 0x00008000;
constexpr uint32_t SCE_CTRL_LTRIGGER = 0x00010000, SCE_CTRL_RTRIGGER = 0x00020000;

enum class GameButton : unsigned char {
    Up, Down, Left, Right, Confirm, Cancel, Inventory, Character, Map,
    Pause, L1, R1, L2, R2, Start, Select, Count
};

struct TouchPoint {
    bool active = false;
    bool pressed = false;
    bool released = false;
    unsigned char id = 0;
    float x = 0.0f;
    float y = 0.0f;
};

class IInputBackend {
public:
    static constexpr std::size_t MaxTouches = 8;
    virtual ~IInputBackend() = default;
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual void update() = 0;
    virtual bool held(GameButton button) const = 0;
    virtual bool pressed(GameButton button) const = 0;
    virtual bool released(GameButton button) const = 0;
    // Transitional adapter: old scene code used Vita bitmasks. Backends map
    // those masks here while gameplay code uses GameButton exclusively.
    bool held(uint32_t vitaMask) const { return held(fromVitaMask(vitaMask)); }
    bool pressed(uint32_t vitaMask) const { return pressed(fromVitaMask(vitaMask)); }
    bool released(uint32_t vitaMask) const { return released(fromVitaMask(vitaMask)); }
    virtual float leftX() const = 0;
    virtual float leftY() const = 0;
    virtual const std::array<TouchPoint, MaxTouches>& touches() const = 0;
    virtual const TouchPoint* primaryTouch() const = 0;
    virtual bool quitRequested() const = 0;
private:
    static GameButton fromVitaMask(uint32_t mask) {
        switch (mask) {
            case 0x00000010: return GameButton::Up; case 0x00000040: return GameButton::Down;
            case 0x00000080: return GameButton::Left; case 0x00000020: return GameButton::Right;
            case 0x00004000: return GameButton::Confirm; case 0x00002000: return GameButton::Cancel;
            case 0x00008000: return GameButton::Inventory; case 0x00001000: return GameButton::Character;
            case 0x00000800: return GameButton::Map; case 0x00000008: return GameButton::Start;
            case 0x00000001: return GameButton::Select; case 0x00000400: return GameButton::L1;
            case 0x00020000: return GameButton::R1; default: return GameButton::Pause;
        }
    }
};

} // namespace pdv
