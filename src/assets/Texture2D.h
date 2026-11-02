#pragma once

#include <cstdint>
#include <string>
#include <vector>

#if defined(PD_PLATFORM_PS3)
namespace pdv { class Renderer2D; }
#else
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Surface;
#endif

namespace pdv {

class Texture2D {
public:
    Texture2D() = default;
    ~Texture2D();

    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    bool loadPng(const std::string& path);
    const std::string& error() const { return error_; }
#if !defined(PD_PLATFORM_PS3)
    bool loadSurface(SDL_Surface* surface);
    void destroy();

    static void setRenderer(SDL_Renderer* renderer);

    bool valid() const {
#if defined(PD_PLATFORM_PSP)
        return texture_ != nullptr || !pages_.empty();
#else
        return texture_ != nullptr;
#endif
    }

    SDL_Texture* native() const { return texture_; }
#if defined(PD_PLATFORM_PSP)
    struct TexturePage {
        SDL_Texture* texture = nullptr;
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
    };
    bool paged() const { return !pages_.empty(); }
    const std::vector<TexturePage>& pages() const { return pages_; }
#endif
#else
    void destroy();

    bool valid() const { return resident_; }
    uint32_t nativeOffset() const { return rsxOffset_; }
    int storageWidth() const { return storageWidth_; }
    int storageHeight() const { return storageHeight_; }
#endif
    int width() const { return width_; }
    int height() const { return height_; }
#if defined(PD_PLATFORM_PS3)
    const uint8_t* rgbaPixels() const { return pixels_.empty() ? nullptr : pixels_.data(); }
#endif

private:
#if !defined(PD_PLATFORM_PS3)
    static SDL_Renderer* renderer_;
    SDL_Texture* texture_ = nullptr;
#if defined(PD_PLATFORM_PSP)
    std::vector<TexturePage> pages_;
#endif
#else
    int storageWidth_ = 0;
    int storageHeight_ = 0;
    uint32_t rsxOffset_ = 0;
    bool resident_ = false;
    std::vector<uint8_t> pixels_;
#endif
    int width_ = 0;
    int height_ = 0;
    std::string error_;
};

} // namespace pdv
