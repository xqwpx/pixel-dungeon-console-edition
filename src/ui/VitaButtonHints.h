#pragma once

#include <initializer_list>
#include <string>

#include "assets/Texture2D.h"
#include "render/BitmapFont.h"
#include "render/Renderer2D.h"

namespace pdv {

// UIButtonsPSVita.png: 16x16 horizontal frames in the order supplied by art.
enum class VitaButtonIcon : int { Circle, Cross, Triangle, Square, DPad, L1, R1 };

struct VitaHintPart {
    VitaButtonIcon icon;
    std::string text;
};

inline void drawVitaButtonHint(Renderer2D& renderer, const BitmapFont& font, const Texture2D& buttons,
                               std::initializer_list<VitaHintPart> parts, float centerX, float y,
                               float textScale = 0.7f, float brightness = 1.0f) {
    constexpr float iconSize = 16.0f;
    constexpr float gap = 3.0f;
    // The font atlas is rendered at its native screen-pixel size.  Fractional
    // scales blur individual glyph pixels after the 480x272 UI is upscaled.
    textScale = 1.0f;
    float width = 0.0f;
    for (const VitaHintPart& part : parts) width += iconSize + gap + font.measure(part.text, textScale) + gap * 2.0f;
    if (width > 0.0f) width -= gap * 2.0f;

    float x = centerX - width * 0.5f;
    for (const VitaHintPart& part : parts) {
        renderer.texture(buttons, x, y - 4.0f, iconSize, iconSize,
            TextureRegion(static_cast<float>(static_cast<int>(part.icon) * 16), 0.0f, 16.0f, 16.0f), 1.0f, brightness);
        x += iconSize + gap;
        font.draw(renderer, part.text, x, y, textScale, brightness, 1.0f);
        x += font.measure(part.text, textScale) + gap * 2.0f;
    }
}

} // namespace pdv
