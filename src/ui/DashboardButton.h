#pragma once

#include <string>

#include "render/Sprite.h"

namespace pdv {

class BitmapFont;
class Renderer2D;
class Texture2D;

class DashboardButton {
public:
    static constexpr float Size = 48.0f;
    static constexpr float ImageSize = 32.0f;

    void initialize(
        Texture2D* dashboardTexture,
        const std::string& label,
        int iconIndex
    );

    void setPosition(float x, float y);
    void setLabel(const std::string& label) { label_ = label; }

    bool contains(float x, float y) const;

    void draw(
        Renderer2D& renderer,
        const BitmapFont& font,
        bool selected,
        bool pressed
    ) const;

    float x() const { return x_; }
    float y() const { return y_; }

private:
    std::string label_;
    Sprite icon_;

    float x_ = 0.0f;
    float y_ = 0.0f;
};

} // namespace pdv
