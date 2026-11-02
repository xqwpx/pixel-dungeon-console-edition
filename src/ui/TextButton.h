#pragma once

#include <string>

#include "render/NinePatch.h"

namespace pdv {

class BitmapFont;
class Renderer2D;
class Texture2D;

class TextButton {
public:
    void initialize(
        Texture2D* chromeTexture,
        BitmapFont* font,
        const std::string& label
    );

    void setRect(float x, float y, float width, float height);
    void setLabel(const std::string& label) { label_ = label; }

    bool contains(float x, float y) const;

    void draw(
        Renderer2D& renderer,
        bool selected = false,
        bool pressed = false,
        bool enabled = true
    ) const;

    float x() const { return x_; }
    float y() const { return y_; }
    float width() const { return width_; }
    float height() const { return height_; }

private:
    BitmapFont* font_ = nullptr;
    std::string label_;

    NinePatch background_;

    float x_ = 0.0f;
    float y_ = 0.0f;
    float width_ = 0.0f;
    float height_ = 0.0f;
};

} // namespace pdv
