#pragma once

namespace pdv {

enum class PS1Button : unsigned char {
    Up, Down, Left, Right, Confirm, Cancel, Inventory, Character, Map,
    Pause, L1, R1, L2, R2, Start, Select, Count
};

class PS1Input final {
public:
    bool initialize();
    void shutdown();
    void update();
    bool held(PS1Button button) const;
    bool pressed(PS1Button button) const;
    bool released(PS1Button button) const;
    float leftX() const { return leftX_; }
    float leftY() const { return leftY_; }

private:
    static constexpr unsigned int ButtonCount = static_cast<unsigned int>(PS1Button::Count);
    alignas(4) unsigned char padBuffers_[2][34]{};
    bool current_[ButtonCount]{};
    bool previous_[ButtonCount]{};
    float leftX_ = 0.0f;
    float leftY_ = 0.0f;
    bool initialized_ = false;
};

} // namespace pdv
