#include "assets/Texture2D.h"
#include "assets/AssetPath.h"

#include <cstdio>
#include <cstring>
#include <vector>

#include <png.h>
#include <SDL2/SDL.h>
#if defined(PD_PLATFORM_PSP)
#include <pspiofilemgr.h>
#endif

namespace pdv {

namespace {

#if defined(PD_PLATFORM_PS3)
int nextPowerOfTwo(int value) {
    int result = 1;
    while (result < value) result <<= 1;
    return result;
}
#endif

#if defined(PD_PLATFORM_PSP)
struct PspPngFile {
    SceUID handle = -1;
};

void readPngFromPsp(png_structp png, png_bytep data, png_size_t length) {
    PspPngFile* file = static_cast<PspPngFile*>(png_get_io_ptr(png));
    if (file == nullptr || sceIoRead(file->handle, data, length) != static_cast<int>(length)) {
        png_error(png, "PSP file read failed");
    }
}
#endif

} // namespace

SDL_Renderer* Texture2D::renderer_ = nullptr;

void Texture2D::setRenderer(SDL_Renderer* renderer) {
    renderer_ = renderer;
}

Texture2D::~Texture2D() {
    destroy();
}

bool Texture2D::loadPng(const std::string& path) {
    destroy();
    error_.clear();

    const std::string resolvedPath = resolveAssetPath(path);
#if defined(PD_PLATFORM_PSP)
    PspPngFile file{sceIoOpen(resolvedPath.c_str(), PSP_O_RDONLY, 0)};
#else
    FILE* file = std::fopen(resolvedPath.c_str(), "rb");
#endif
#if defined(PD_PLATFORM_PSP)
    if (file.handle < 0) {
#else
    if (file == nullptr) {
#endif
        error_ = "cannot open " + resolvedPath;
        return false;
    }

    const auto closeFile = [&file]() {
#if defined(PD_PLATFORM_PSP)
        sceIoClose(file.handle);
#else
        std::fclose(file);
#endif
    };

    png_byte signature[8];
#if defined(PD_PLATFORM_PSP)
    const int result = sceIoRead(file.handle, signature, sizeof(signature));
    const std::size_t signatureBytes = result > 0 ? static_cast<std::size_t>(result) : 0;
#else
    const std::size_t signatureBytes = std::fread(signature, 1, sizeof(signature), file);
#endif
    if (signatureBytes != sizeof(signature) ||
        png_sig_cmp(signature, 0, sizeof(signature)) != 0) {
        char diagnostic[96]{};
        std::snprintf(diagnostic, sizeof(diagnostic),
                      "invalid PNG (%u bytes, %02X %02X %02X %02X) ",
                      static_cast<unsigned int>(signatureBytes), signature[0], signature[1],
                      signature[2], signature[3]);
        error_ = std::string(diagnostic) + resolvedPath;
        closeFile();
        return false;
    }

    png_structp png = png_create_read_struct(
        PNG_LIBPNG_VER_STRING,
        nullptr,
        nullptr,
        nullptr
    );

    if (png == nullptr) {
        error_ = "libpng init failed";
        closeFile();
        return false;
    }

    png_infop info = png_create_info_struct(png);
    if (info == nullptr) {
        error_ = "libpng info failed";
        png_destroy_read_struct(&png, nullptr, nullptr);
        closeFile();
        return false;
    }

    if (setjmp(png_jmpbuf(png))) {
        error_ = "libpng decode failed";
        png_destroy_read_struct(&png, &info, nullptr);
        closeFile();
        return false;
    }

#if defined(PD_PLATFORM_PSP)
    png_set_read_fn(png, &file, readPngFromPsp);
#else
    png_init_io(png, file);
#endif
    png_set_sig_bytes(png, sizeof(signature));
    png_read_info(png, info);

    width_ = static_cast<int>(png_get_image_width(png, info));
    height_ = static_cast<int>(png_get_image_height(png, info));

    const png_byte colorType = png_get_color_type(png, info);
    const png_byte bitDepth = png_get_bit_depth(png, info);

    if (bitDepth == 16) {
        png_set_strip_16(png);
    }

    if (colorType == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png);
    }

    if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8) {
        png_set_expand_gray_1_2_4_to_8(png);
    }

    const bool hasTransparency =
        colorType == PNG_COLOR_TYPE_RGBA ||
        colorType == PNG_COLOR_TYPE_GRAY_ALPHA ||
        png_get_valid(png, info, PNG_INFO_tRNS);

    if (png_get_valid(png, info, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png);
    }

    if (!hasTransparency) {
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    }

    if (colorType == PNG_COLOR_TYPE_GRAY ||
        colorType == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png);
    }

    png_read_update_info(png, info);

    const png_size_t rowBytes = png_get_rowbytes(png, info);
    std::vector<uint8_t> pixels(rowBytes * static_cast<std::size_t>(height_));

    std::vector<png_bytep> rows(static_cast<std::size_t>(height_));
    for (int y = 0; y < height_; ++y) {
        rows[static_cast<std::size_t>(y)] =
            pixels.data() +
            static_cast<std::size_t>(y) *
            rowBytes;
    }

    png_read_image(png, rows.data());
    png_read_end(png, nullptr);

    png_destroy_read_struct(&png, &info, nullptr);
    closeFile();

#if defined(PD_PLATFORM_PS2)
    // gsKit's SDL backend uploads the byte stream directly to PSM_CT32; keep
    // libpng's RGBA channel order. GS blending uses alpha in the 0..128
    // range, so only scale the alpha component.
    for (std::size_t offset = 3; offset < pixels.size(); offset += 4) {
        pixels[offset] = static_cast<uint8_t>(
            (static_cast<unsigned int>(pixels[offset]) + 1u) >> 1u);
    }
#endif

    if (renderer_ == nullptr) {
        error_ = "renderer unavailable";
        width_ = 0;
        height_ = 0;
        return false;
    }

    const int imageWidth = width_;
    const int imageHeight = height_;
    const uint8_t* surfacePixels = pixels.data();
    int surfaceWidth = imageWidth;
    int surfaceHeight = imageHeight;
    int surfacePitch = static_cast<int>(rowBytes);

#if defined(PD_PLATFORM_PSP)
    // The PSP GU backend accepts textures no larger than 512x512. Keep atlas
    // coordinates in their original space and upload oversized PNGs as pages.
    constexpr int PspTextureLimit = 512;
    if (imageWidth > PspTextureLimit || imageHeight > PspTextureLimit) {
        for (int pageY = 0; pageY < imageHeight; pageY += PspTextureLimit) {
            for (int pageX = 0; pageX < imageWidth; pageX += PspTextureLimit) {
                const int pageWidth = std::min(PspTextureLimit, imageWidth - pageX);
                const int pageHeight = std::min(PspTextureLimit, imageHeight - pageY);
                SDL_Texture* page = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ABGR8888,
                                                      SDL_TEXTUREACCESS_STATIC,
                                                      pageWidth, pageHeight);
                if (page == nullptr ||
                    SDL_UpdateTexture(page, nullptr,
                                      pixels.data() + static_cast<std::size_t>(pageY) * rowBytes +
                                      static_cast<std::size_t>(pageX) * 4u,
                                      static_cast<int>(rowBytes)) != 0) {
                    if (page != nullptr) SDL_DestroyTexture(page);
                    destroy();
                    error_ = "PSP texture page upload failed: " + std::string(SDL_GetError());
                    return false;
                }
                SDL_SetTextureBlendMode(page, SDL_BLENDMODE_BLEND);
                SDL_SetTextureScaleMode(page, SDL_ScaleModeNearest);
                pages_.push_back({page, pageX, pageY, pageWidth, pageHeight});
            }
        }
        return true;
    }
#endif

#if defined(PD_PLATFORM_PS3)
    // SDL's PSL1GHT/RSX backend cannot create non-power-of-two textures.
    // Keep the logical image size below, but upload a transparent POT texture.
    const int paddedWidth = nextPowerOfTwo(imageWidth);
    const int paddedHeight = nextPowerOfTwo(imageHeight);
    std::vector<uint8_t> paddedPixels(
        static_cast<std::size_t>(paddedWidth) * static_cast<std::size_t>(paddedHeight) * 4u,
        0u);
    for (int y = 0; y < imageHeight; ++y) {
        std::memcpy(paddedPixels.data() + static_cast<std::size_t>(y) * paddedWidth * 4u,
                    pixels.data() + static_cast<std::size_t>(y) * rowBytes,
                    static_cast<std::size_t>(imageWidth) * 4u);
    }
    surfacePixels = paddedPixels.data();
    surfaceWidth = paddedWidth;
    surfaceHeight = paddedHeight;
    surfacePitch = paddedWidth * 4;
#endif

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(
        const_cast<uint8_t*>(surfacePixels),
        surfaceWidth,
        surfaceHeight,
        32,
        surfacePitch,
        SDL_PIXELFORMAT_RGBA32
    );

    if (surface == nullptr) {
        error_ = "SDL surface failed";
        width_ = 0;
        height_ = 0;
        return false;
    }

    const bool loaded = loadSurface(surface);
    SDL_FreeSurface(surface);
    if (loaded) {
        // Game coordinates and atlas regions refer to the original PNG, not
        // the PS3-only transparent padding used for the GPU upload.
        width_ = imageWidth;
        height_ = imageHeight;
    }
    if (!loaded && error_.empty()) error_ = "SDL texture failed";
    return loaded;
}

bool Texture2D::loadSurface(SDL_Surface* surface) {
    destroy();
    if (renderer_ == nullptr || surface == nullptr) { error_ = "renderer/surface unavailable"; return false; }
#if defined(PD_PLATFORM_PS2) || defined(PD_PLATFORM_PSP)
    // The gsKit renderer natively advertises ABGR8888. Avoid SDL's generic
    // CreateTextureFromSurface conversion, which loses PNG transparency on
    // this backend despite the source surface already being RGBA bytes.
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ABGR8888,
                                 SDL_TEXTUREACCESS_STATIC, surface->w, surface->h);
    if (texture_ == nullptr || SDL_UpdateTexture(texture_, nullptr, surface->pixels, surface->pitch) != 0) {
        error_ = "texture upload failed: " + std::string(SDL_GetError());
        if (texture_ != nullptr) { SDL_DestroyTexture(texture_); texture_ = nullptr; }
        return false;
    }
#else
    texture_ = SDL_CreateTextureFromSurface(renderer_, surface);
    if (texture_ == nullptr) { error_ = "SDL_CreateTextureFromSurface failed: " + std::string(SDL_GetError()); return false; }
#endif
    width_ = surface->w;
    height_ = surface->h;
    SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(texture_, SDL_ScaleModeNearest);
    return true;
}

void Texture2D::destroy() {
    if (texture_ != nullptr) {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }
#if defined(PD_PLATFORM_PSP)
    for (const TexturePage& page : pages_) {
        if (page.texture != nullptr) SDL_DestroyTexture(page.texture);
    }
    pages_.clear();
#endif

    width_ = 0;
    height_ = 0;
}

} // namespace pdv
