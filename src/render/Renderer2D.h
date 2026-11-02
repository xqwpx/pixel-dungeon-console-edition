#pragma once

#include <cstdint>
#if defined(PD_PLATFORM_PS3)
#include <memory>
#endif

#include "render/TextureRegion.h"

#if !defined(PD_PLATFORM_PS3)
struct SDL_Window;
struct SDL_Renderer;
#endif

namespace pdv {

class Texture2D;
#if defined(PD_PLATFORM_PS3)
class PS3Renderer2D;
#endif

class Renderer2D {
public:
#if defined(PD_PLATFORM_PSP)
    // The PSP framebuffer is already the game's original 480x272 layout.
    // Rendering at Vita resolution makes the SDL PSP backend clip/scissor
    // sprites instead of performing the intended downscale.
    static constexpr int ScreenWidth = 480;
    static constexpr int ScreenHeight = 272;
    static constexpr int LogicalWidth = 480;
    static constexpr int LogicalHeight = 272;
#else
    static constexpr int ScreenWidth = 960;
    static constexpr int ScreenHeight = 544;

    // We draw the game at half-resolution and scale it 2x to the Vita panel.
    // This keeps original Pixel Dungeon UI dimensions meaningful.
    static constexpr int LogicalWidth = 480;
    static constexpr int LogicalHeight = 272;
#endif

    Renderer2D();
    ~Renderer2D();

    bool initialize();
    void shutdown();

    void beginFrame();
    void present();

    void rect(float x, float y, float w, float h, uint32_t rgba);
    void outlineRect(
        float x,
        float y,
        float w,
        float h,
        float thickness,
        uint32_t rgba
    );

    void texture(
        const Texture2D& texture,
        float x,
        float y,
        float w,
        float h,
        const TextureRegion& region,
        float alpha = 1.0f,
        float brightness = 1.0f,
        bool flipX = false,
        uint32_t tint = 0xFFFFFFFF
    );

    void fullTexture(
        const Texture2D& texture,
        float x,
        float y,
        float w,
        float h,
        float alpha = 1.0f,
        float brightness = 1.0f
    );

    void tiled(
        const Texture2D& texture,
        float x,
        float y,
        float w,
        float h,
        float offsetX,
        float offsetY,
        float alpha = 1.0f
    );

    void setAdditiveBlend(bool enabled);

private:
#if defined(PD_PLATFORM_PS3)
    std::unique_ptr<PS3Renderer2D> ps3_;
#else
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
#endif
    bool additiveBlend_ = false;
};

} // namespace pdv
