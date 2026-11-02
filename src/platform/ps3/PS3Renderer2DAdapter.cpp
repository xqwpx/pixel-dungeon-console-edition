#include "render/Renderer2D.h"

#include "assets/Texture2D.h"
#include "platform/ps3/PS3Renderer2D.h"

#include <algorithm>
#include <cmath>

namespace pdv {

Renderer2D::Renderer2D() = default;
Renderer2D::~Renderer2D() = default;
bool Renderer2D::initialize() { ps3_ = std::make_unique<PS3Renderer2D>(); if (!ps3_->initialize()) { ps3_.reset(); return false; } return true; }
void Renderer2D::shutdown() { if (ps3_) ps3_->shutdown(); ps3_.reset(); }
void Renderer2D::beginFrame() { if (ps3_) ps3_->beginFrame(); }
void Renderer2D::present() { if (ps3_) ps3_->present(); }
void Renderer2D::rect(float x, float y, float w, float h, uint32_t rgba) { if (ps3_) ps3_->rect(x, y, w, h, rgba, additiveBlend_); }
void Renderer2D::outlineRect(float x, float y, float w, float h, float thickness, uint32_t rgba) { rect(x, y, w, thickness, rgba); rect(x, y + h - thickness, w, thickness, rgba); rect(x, y, thickness, h, rgba); rect(x + w - thickness, y, thickness, h, rgba); }
void Renderer2D::texture(const Texture2D& textureObject, float x, float y, float w, float h, const TextureRegion& region, float alpha, float brightness, bool flipX, uint32_t tint) { if (ps3_) ps3_->texture(textureObject, x, y, w, h, region, alpha, brightness, flipX, tint, additiveBlend_); }
void Renderer2D::fullTexture(const Texture2D& textureObject, float x, float y, float w, float h, float alpha, float brightness) { texture(textureObject, x, y, w, h, TextureRegion(0.f, 0.f, static_cast<float>(textureObject.width()), static_cast<float>(textureObject.height())), alpha, brightness); }
void Renderer2D::tiled(const Texture2D& textureObject, float x, float y, float w, float h, float offsetX, float offsetY, float alpha) {
    if (!textureObject.valid()) return;
    const float tw = static_cast<float>(textureObject.width()), th = static_cast<float>(textureObject.height());
    for (float py = y + std::fmod(offsetY, th) - (std::fmod(offsetY, th) > 0 ? th : 0); py < y + h; py += th)
        for (float px = x + std::fmod(offsetX, tw) - (std::fmod(offsetX, tw) > 0 ? tw : 0); px < x + w; px += tw) {
            const float l = std::max(px, x), t = std::max(py, y), r = std::min(px + tw, x + w), b = std::min(py + th, y + h);
            if (r > l && b > t) texture(textureObject, l, t, r - l, b - t, TextureRegion(l - px, t - py, r - l, b - t), alpha);
        }
}
void Renderer2D::setAdditiveBlend(bool enabled) { additiveBlend_ = enabled; }

} // namespace pdv
