#include "render/Renderer2D.h"

#include "assets/Texture2D.h"

#include <algorithm>
#include <cmath>
#if defined(PD_PLATFORM_PS2)
#include <cstdio>
#endif

#include <SDL2/SDL.h>

namespace pdv {
namespace {

int rounded(float value) {
    return static_cast<int>(std::floor(value + 0.5f));
}

Uint8 colorByte(uint32_t rgba, int shift) {
    return static_cast<Uint8>((rgba >> shift) & 0xFFu);
}

} // namespace

Renderer2D::Renderer2D() = default;
Renderer2D::~Renderer2D() = default;

bool Renderer2D::initialize() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
#if defined(PD_PLATFORM_PS2)
        std::printf("[PDPS2] SDL_Init: %s\n", SDL_GetError());
#endif
        return false;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    window_ = SDL_CreateWindow(
        "PIXEL DUNGEON CONSOLE EDITION",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        ScreenWidth,
        ScreenHeight,
        // A resizable desktop window gives the standard maximise button the
        // expected full-screen desktop behaviour. Logical rendering keeps the
        // original pixel layout at every window size.
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (window_ == nullptr) {
#if defined(PD_PLATFORM_PS2)
        std::printf("[PDPS2] SDL_CreateWindow: %s\n", SDL_GetError());
#endif
        shutdown();
        return false;
    }

#if defined(PD_PLATFORM_PSP)
    // SDL's PSP backend does not reliably implement the desktop VSync flag:
    // after the first few presents it can block the game thread indefinitely.
    // Let the PSP backend present the framebuffer on its own schedule.
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
#else
    renderer_ = SDL_CreateRenderer(
        window_,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
#endif

    if (renderer_ == nullptr) {
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    }

    if (renderer_ == nullptr) {
#if defined(PD_PLATFORM_PS2)
        std::printf("[PDPS2] SDL_CreateRenderer: %s\n", SDL_GetError());
#endif
        shutdown();
        return false;
    }

    if (SDL_RenderSetLogicalSize(renderer_, LogicalWidth, LogicalHeight) != 0) {
#if defined(PD_PLATFORM_PS2)
        std::printf("[PDPS2] SDL_RenderSetLogicalSize: %s\n", SDL_GetError());
#endif
        shutdown();
        return false;
    }

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    Texture2D::setRenderer(renderer_);
    return true;
}

void Renderer2D::shutdown() {
    Texture2D::setRenderer(nullptr);

    if (renderer_ != nullptr) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }

    if (window_ != nullptr) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

    SDL_Quit();
}

void Renderer2D::beginFrame() {
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
    setAdditiveBlend(false);
}

void Renderer2D::present() {
    SDL_RenderPresent(renderer_);
}

void Renderer2D::rect(float x, float y, float w, float h, uint32_t rgba) {
    SDL_SetRenderDrawBlendMode(
        renderer_,
        additiveBlend_ ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND
    );
    SDL_SetRenderDrawColor(
        renderer_,
        colorByte(rgba, 24),
        colorByte(rgba, 16),
        colorByte(rgba, 8),
        colorByte(rgba, 0)
    );
    const SDL_Rect dst{rounded(x), rounded(y), rounded(w), rounded(h)};
    SDL_RenderFillRect(renderer_, &dst);
}

void Renderer2D::outlineRect(
    float x,
    float y,
    float w,
    float h,
    float thickness,
    uint32_t rgba
) {
    rect(x, y, w, thickness, rgba);
    rect(x, y + h - thickness, w, thickness, rgba);
    rect(x, y, thickness, h, rgba);
    rect(x + w - thickness, y, thickness, h, rgba);
}

void Renderer2D::texture(
    const Texture2D& textureObject,
    float x,
    float y,
    float w,
    float h,
    const TextureRegion& region,
    float alpha,
    float brightness,
    bool flipX,
    uint32_t tint
) {
    if (!textureObject.valid()) {
        return;
    }

    SDL_Texture* texture = textureObject.native();
    const float clampedBrightness = std::max(0.0f, std::min(1.0f, brightness));
    const float clampedAlpha = std::max(0.0f, std::min(1.0f, alpha));
    const auto tintByte=[&](int shift){return static_cast<Uint8>((((tint>>shift)&0xFFu)*clampedBrightness));};
    const auto configureTexture = [&](SDL_Texture* value) {
        SDL_SetTextureColorMod(value, tintByte(24), tintByte(16), tintByte(8));
        SDL_SetTextureAlphaMod(value, static_cast<Uint8>(clampedAlpha * 255.0f));
        SDL_SetTextureBlendMode(value, additiveBlend_ ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);
    };

#if defined(PD_PLATFORM_PSP)
    if (textureObject.paged()) {
        const float regionRight = region.x + region.w;
        const float regionBottom = region.y + region.h;
        for (const Texture2D::TexturePage& page : textureObject.pages()) {
            const float left = std::max(region.x, static_cast<float>(page.x));
            const float top = std::max(region.y, static_cast<float>(page.y));
            const float right = std::min(regionRight, static_cast<float>(page.x + page.width));
            const float bottom = std::min(regionBottom, static_cast<float>(page.y + page.height));
            if (right <= left || bottom <= top) continue;
            const float partW = right - left;
            const float partH = bottom - top;
            const float sourceOffsetX = (left - region.x) / region.w;
            const float sourceOffsetY = (top - region.y) / region.h;
            const float destX = flipX ? x + w * (1.0f - sourceOffsetX - partW / region.w)
                                      : x + w * sourceOffsetX;
            const SDL_Rect src{rounded(left - page.x), rounded(top - page.y), rounded(partW), rounded(partH)};
            const SDL_Rect dst{rounded(destX), rounded(y + h * sourceOffsetY),
                               rounded(w * partW / region.w), rounded(h * partH / region.h)};
            configureTexture(page.texture);
            if (flipX) SDL_RenderCopyEx(renderer_, page.texture, &src, &dst, 0.0, nullptr, SDL_FLIP_HORIZONTAL);
            else SDL_RenderCopy(renderer_, page.texture, &src, &dst);
        }
        return;
    }
#endif

    configureTexture(texture);

    const SDL_Rect src{
        rounded(region.x), rounded(region.y), rounded(region.w), rounded(region.h)
    };
    const SDL_Rect dst{rounded(x), rounded(y), rounded(w), rounded(h)};
    if (flipX) {
        SDL_RenderCopyEx(
            renderer_, texture, &src, &dst, 0.0, nullptr,
            SDL_FLIP_HORIZONTAL
        );
    } else {
        SDL_RenderCopy(renderer_, texture, &src, &dst);
    }
}

void Renderer2D::fullTexture(
    const Texture2D& textureObject,
    float x,
    float y,
    float w,
    float h,
    float alpha,
    float brightness
) {
    texture(
        textureObject,
        x,
        y,
        w,
        h,
        TextureRegion(0.0f, 0.0f,
            static_cast<float>(textureObject.width()),
            static_cast<float>(textureObject.height())),
        alpha,
        brightness
    );
}

void Renderer2D::tiled(
    const Texture2D& textureObject,
    float x,
    float y,
    float w,
    float h,
    float offsetX,
    float offsetY,
    float alpha
) {
    if (!textureObject.valid()) return;

    const float tw = static_cast<float>(textureObject.width());
    const float th = static_cast<float>(textureObject.height());
    if (tw <= 0.0f || th <= 0.0f) return;

    float startX = x + std::fmod(offsetX, tw);
    float startY = y + std::fmod(offsetY, th);
    if (startX > x) startX -= tw;
    if (startY > y) startY -= th;

    const float endX = x + w;
    const float endY = y + h;
    for (float py = startY; py < endY; py += th) {
        for (float px = startX; px < endX; px += tw) {
            const float clipL = std::max(px, x);
            const float clipT = std::max(py, y);
            const float clipR = std::min(px + tw, endX);
            const float clipB = std::min(py + th, endY);
            if (clipR <= clipL || clipB <= clipT) continue;

            texture(
                textureObject,
                clipL,
                clipT,
                clipR - clipL,
                clipB - clipT,
                TextureRegion(clipL - px, clipT - py, clipR - clipL, clipB - clipT),
                alpha
            );
        }
    }
}

void Renderer2D::setAdditiveBlend(bool enabled) {
    additiveBlend_ = enabled;
}

} // namespace pdv
