#include "ui/DashboardButton.h"

#include "render/BitmapFont.h"
#include "render/Renderer2D.h"

namespace pdv {

void DashboardButton::initialize(
    Texture2D* dashboardTexture,
    const std::string& label,
    int iconIndex
) {
    label_ = label;

    // SDL's PS2 renderer samples the first texel column of this atlas
    // incorrectly when a source rect starts at U=0.  The column is a
    // transparent gutter, so skip it only for the first dashboard tile.
    const float sourceX = iconIndex == 0 ? 1.0f : static_cast<float>(iconIndex) * ImageSize;
    const float sourceWidth = iconIndex == 0 ? ImageSize - 1.0f : ImageSize;

    icon_ = Sprite(
        dashboardTexture,
        TextureRegion(
            sourceX,
            0.0f,
            sourceWidth,
            ImageSize
        )
    );
}

void DashboardButton::setPosition(float x, float y) {
    x_ = x;
    y_ = y;

    icon_.setPosition(
        x_ + (Size - ImageSize) * 0.5f,
        y_
    );
}

bool DashboardButton::contains(float x, float y) const {
    return x >= x_ &&
           y >= y_ &&
           x < x_ + Size &&
           y < y_ + Size;
}

void DashboardButton::draw(
    Renderer2D& renderer,
    const BitmapFont& font,
    bool selected,
    bool pressed
) const {
    if (selected) {
        renderer.outlineRect(
            x_ + 2.0f,
            y_ - 2.0f,
            Size - 4.0f,
            Size - 2.0f,
            1.0f,
            0xD8D8D8C0
        );
    }

    Sprite icon = icon_;
    icon.setBrightness(pressed ? 1.55f : (selected ? 1.25f : 1.0f));
    icon.draw(renderer);

    font.drawCentered(
        renderer,
        label_,
        x_ + Size * 0.5f,
        y_ + ImageSize + 2.0f,
        1.0f,
        selected ? 1.25f : 1.0f,
        1.0f
    );
}

} // namespace pdv
