#include "ui/TextButton.h"

#include "render/BitmapFont.h"
#include "render/Renderer2D.h"

namespace pdv {

void TextButton::initialize(
    Texture2D* chromeTexture,
    BitmapFont* font,
    const std::string& label
) {
    font_ = font;
    label_ = label;

    // Pixel Dungeon Chrome.Type.BUTTON:
    // chrome.png region x=58, y=0, 6x6, border=2.
    background_.configure(
        chromeTexture,
        TextureRegion(58.0f, 0.0f, 6.0f, 6.0f),
        2.0f,
        2.0f,
        2.0f,
        2.0f
    );
}

void TextButton::setRect(
    float x,
    float y,
    float width,
    float height
) {
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
}

bool TextButton::contains(float x, float y) const {
    return x >= x_ &&
           y >= y_ &&
           x < x_ + width_ &&
           y < y_ + height_;
}

void TextButton::draw(
    Renderer2D& renderer,
    bool selected,
    bool pressed,
    bool enabled
) const {
    const float brightness =
        !enabled ? 0.42f :
        pressed ? 1.35f :
        selected ? 1.15f :
        1.0f;

    background_.draw(
        renderer,
        x_,
        y_,
        width_,
        height_,
        brightness,
        1.0f
    );

    if (selected) {
        renderer.outlineRect(
            x_ - 1.0f,
            y_ - 1.0f,
            width_ + 2.0f,
            height_ + 2.0f,
            1.0f,
            0xFFFF88D8
        );
    }

    if (font_ != nullptr) {
        font_->drawCentered(
            renderer,
            label_,
            x_ + width_ * 0.5f,
            y_ + (height_ - 8.0f) * 0.5f,
            1.0f,
            enabled ? (selected ? 1.25f : 1.0f) : 0.38f,
            1.0f
        );
    }
}

} // namespace pdv
