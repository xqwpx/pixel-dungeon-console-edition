#include "render/BitmapFont.h"

#include "assets/AssetCatalog.h"
#include "assets/AssetPath.h"
#include "assets/Texture2D.h"
#include "render/Renderer2D.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iterator>

namespace pdv {
namespace {

// The UI layout was authored around an 8px font.  The new atlas is 16px for
// cleaner source pixels, so render it at half its atlas size by default.
constexpr float LayoutScale = 0.5f;

// The 480x272 logical canvas is doubled on the Vita/desktop output.  A
// fractional requested font scale therefore resamples the 1px atlas and makes
// its strokes uneven.  Restrict every text draw to whole source pixels.
float pixelScale(float scale) {
    return std::max(1.0f, std::floor(scale + 0.5f));
}

bool attribute(const std::string& element, const char* name, int& value) {
    const std::string key = std::string(name) + "=\"";
    const std::size_t begin = element.find(key);
    if (begin == std::string::npos) return false;
    const char* number = element.c_str() + begin + key.size();
    char* end = nullptr;
    value = static_cast<int>(std::strtol(number, &end, 10));
    return end != number;
}

} // namespace

bool BitmapFont::initialize(Texture2D* texture) {
    texture_ = texture;
    glyphs_.clear();
    if (!texture_ || !texture_->valid()) return false;

    std::ifstream stream(resolveAssetPath(Assets::UIFontMetrics), std::ios::binary);
    if (!stream) return false;
    const std::string xml((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

    const std::size_t common = xml.find("<common ");
    if (common != std::string::npos) {
        const std::size_t close = xml.find('>', common);
        int parsedLineHeight = 0;
        if (close != std::string::npos && attribute(xml.substr(common, close - common), "lineHeight", parsedLineHeight)) {
            lineHeight_ = parsedLineHeight;
        }
    }

    std::size_t cursor = 0;
    while ((cursor = xml.find("<char ", cursor)) != std::string::npos) {
        const std::size_t close = xml.find("/>", cursor);
        if (close == std::string::npos) break;
        const std::string element = xml.substr(cursor, close - cursor);
        int id = 0;
        Glyph parsed;
        if (attribute(element, "id", id) && attribute(element, "x", parsed.x) &&
            attribute(element, "y", parsed.y) && attribute(element, "width", parsed.width) &&
            attribute(element, "height", parsed.height) && attribute(element, "xoffset", parsed.xOffset) &&
            attribute(element, "yoffset", parsed.yOffset) && attribute(element, "xadvance", parsed.xAdvance)) {
            glyphs_.emplace(static_cast<uint32_t>(id), parsed);
        }
        cursor = close + 2;
    }
    return !glyphs_.empty();
}

const BitmapFont::Glyph* BitmapFont::glyph(uint32_t codepoint) const {
    const auto found = glyphs_.find(codepoint);
    if (found != glyphs_.end()) return &found->second;
    const auto fallback = glyphs_.find(static_cast<uint32_t>('?'));
    return fallback == glyphs_.end() ? nullptr : &fallback->second;
}

uint32_t BitmapFont::nextCodepoint(const std::string& text, std::size_t& offset) {
    const unsigned char first = static_cast<unsigned char>(text[offset++]);
    if (first < 0x80) return first;
    int extra = first < 0xE0 ? 1 : (first < 0xF0 ? 2 : 3);
    uint32_t codepoint = first & (0x7Fu >> extra);
    while (extra-- > 0 && offset < text.size()) {
        const unsigned char next = static_cast<unsigned char>(text[offset++]);
        if ((next & 0xC0u) != 0x80u) return static_cast<uint32_t>('?');
        codepoint = (codepoint << 6u) | (next & 0x3Fu);
    }
    return codepoint;
}

float BitmapFont::measure(const std::string& text, float scale) const {
    scale = pixelScale(scale) * LayoutScale;
    float width = 0.0f;
    float widest = 0.0f;
    for (std::size_t offset = 0; offset < text.size();) {
        const uint32_t codepoint = nextCodepoint(text, offset);
        if (codepoint == '\n') { widest = std::max(widest, width); width = 0.0f; continue; }
        if (const Glyph* current = glyph(codepoint)) width += current->xAdvance * scale;
    }
    return std::max(widest, width);
}

void BitmapFont::draw(Renderer2D& renderer, const std::string& text, float x, float y,
                      float scale, float brightness, float alpha) const {
    if (!texture_ || text.empty()) return;
    scale = pixelScale(scale) * LayoutScale;
    const float originX = x;
    for (std::size_t offset = 0; offset < text.size();) {
        const uint32_t codepoint = nextCodepoint(text, offset);
        if (codepoint == '\n') { x = originX; y += lineHeight_ * scale; continue; }
        const Glyph* current = glyph(codepoint);
        if (!current) continue;
        if (current->width > 0 && current->height > 0) {
            renderer.texture(*texture_, x + current->xOffset * scale, y + current->yOffset * scale,
                current->width * scale, current->height * scale,
                TextureRegion(static_cast<float>(current->x), static_cast<float>(current->y),
                    static_cast<float>(current->width), static_cast<float>(current->height)), alpha, brightness);
        }
        x += current->xAdvance * scale;
    }
}

void BitmapFont::drawCentered(Renderer2D& renderer, const std::string& text, float centerX, float y,
                              float scale, float brightness, float alpha) const {
    draw(renderer, text, centerX - measure(text, scale) * 0.5f, y, scale, brightness, alpha);
}

void BitmapFont::drawColored(Renderer2D& renderer, const std::string& text, float x, float y,
                             uint32_t tint, float scale, float alpha) const {
    if (!texture_ || text.empty()) return;
    scale = pixelScale(scale) * LayoutScale;
    const float originX = x;
    for (std::size_t offset = 0; offset < text.size();) {
        const uint32_t codepoint = nextCodepoint(text, offset);
        if (codepoint == '\n') { x = originX; y += lineHeight_ * scale; continue; }
        const Glyph* current = glyph(codepoint);
        if (!current) continue;
        renderer.texture(*texture_, x + current->xOffset * scale, y + current->yOffset * scale,
            current->width * scale, current->height * scale,
            TextureRegion(static_cast<float>(current->x), static_cast<float>(current->y),
                static_cast<float>(current->width), static_cast<float>(current->height)), alpha, 1.0f, false, tint);
        x += current->xAdvance * scale;
    }
}

} // namespace pdv
