#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace pdv {

class Renderer2D;
class Texture2D;

class BitmapFont {
public:
    bool initialize(Texture2D* texture);

    float measure(const std::string& text, float scale = 1.0f) const;

    void draw(
        Renderer2D& renderer,
        const std::string& text,
        float x,
        float y,
        float scale = 1.0f,
        float brightness = 1.0f,
        float alpha = 1.0f
    ) const;

    void drawCentered(
        Renderer2D& renderer,
        const std::string& text,
        float centerX,
        float y,
        float scale = 1.0f,
        float brightness = 1.0f,
        float alpha = 1.0f
    ) const;

    void drawColored(Renderer2D& renderer,const std::string& text,float x,float y,
                     uint32_t tint,float scale=1.0f,float alpha=1.0f) const;

private:
    struct Glyph {
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        int xOffset = 0;
        int yOffset = 0;
        int xAdvance = 0;
    };

    const Glyph* glyph(uint32_t codepoint) const;
    static uint32_t nextCodepoint(const std::string& text, std::size_t& offset);

    Texture2D* texture_ = nullptr;
    std::unordered_map<uint32_t, Glyph> glyphs_;
    int lineHeight_ = 16;
};

} // namespace pdv
