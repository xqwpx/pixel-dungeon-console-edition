#include "assets/Texture2D.h"

#include "assets/AssetPath.h"
#include "platform/ps3/PS3TextureAllocator.h"

#include <cstdio>
#include <vector>

#include <png.h>

namespace pdv {
namespace {

int nextPowerOfTwo(int value) {
    int result = 1;
    while (result < value) result <<= 1;
    return result;
}

} // namespace

Texture2D::~Texture2D() {
    destroy();
}

bool Texture2D::loadPng(const std::string& path) {
    destroy();
    FILE* file = std::fopen(resolveAssetPath(path).c_str(), "rb");
    if (file == nullptr) return false;

    png_byte signature[8];
    if (std::fread(signature, 1, sizeof(signature), file) != sizeof(signature) ||
        png_sig_cmp(signature, 0, sizeof(signature)) != 0) {
        std::fclose(file);
        return false;
    }
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    png_infop info = png ? png_create_info_struct(png) : nullptr;
    if (png == nullptr || info == nullptr) {
        if (png != nullptr) png_destroy_read_struct(&png, nullptr, nullptr);
        std::fclose(file);
        return false;
    }
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, nullptr);
        std::fclose(file);
        return false;
    }

    png_init_io(png, file);
    png_set_sig_bytes(png, sizeof(signature));
    png_read_info(png, info);
    width_ = static_cast<int>(png_get_image_width(png, info));
    height_ = static_cast<int>(png_get_image_height(png, info));
    const png_byte colorType = png_get_color_type(png, info);
    const png_byte bitDepth = png_get_bit_depth(png, info);
    if (bitDepth == 16) png_set_strip_16(png);
    if (colorType == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png);
    if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8) png_set_expand_gray_1_2_4_to_8(png);
    if (png_get_valid(png, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png);
    if (colorType != PNG_COLOR_TYPE_RGBA && colorType != PNG_COLOR_TYPE_GRAY_ALPHA &&
        !png_get_valid(png, info, PNG_INFO_tRNS)) png_set_filler(png, 0xff, PNG_FILLER_AFTER);
    if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png);
    png_read_update_info(png, info);

    const int originalWidth = width_;
    const int originalHeight = height_;
    const png_size_t rowBytes = png_get_rowbytes(png, info);
    storageWidth_ = nextPowerOfTwo(width_);
    storageHeight_ = nextPowerOfTwo(height_);
    const std::size_t size = static_cast<std::size_t>(storageWidth_) * storageHeight_ * 4u;
    std::vector<uint8_t> pixels(size, 0u);
    std::vector<png_bytep> rows(static_cast<std::size_t>(height_));
    for (int y = 0; y < height_; ++y) {
        rows[static_cast<std::size_t>(y)] = pixels.data() +
            static_cast<std::size_t>(y) * storageWidth_ * 4u;
    }
    png_read_image(png, rows.data());
    png_read_end(png, nullptr);
    png_destroy_read_struct(&png, &info, nullptr);
    std::fclose(file);
    // Retain decoded RGBA for the software PS3 compositor.  The RSX upload
    // below receives its own hardware-specific byte order.
    pixels_ = pixels;
    // libpng yields RGBA bytes.  The PPU is big-endian while the RSX
    // A8R8G8B8 texture format consumes an ARGB 32-bit word, so the in-memory
    // byte order must be A,R,G,B.  Leaving it as RGBA makes the red channel
    // act as alpha and turns most atlas pixels transparent or striped.
    for (int y = 0; y < height_; ++y) {
        uint8_t* row = pixels.data() + static_cast<std::size_t>(y) * storageWidth_ * 4u;
        for (int x = 0; x < width_; ++x) {
            uint8_t* pixel = row + static_cast<std::size_t>(x) * 4u;
            const uint8_t red = pixel[0];
            const uint8_t green = pixel[1];
            const uint8_t blue = pixel[2];
            const uint8_t alpha = pixel[3];
            pixel[0] = alpha;
            pixel[1] = red;
            pixel[2] = green;
            pixel[3] = blue;
        }
    }
    if (!ps3UploadTexture(pixels.data(), static_cast<uint32_t>(size), &rsxOffset_)) {
        destroy();
        return false;
    }
    resident_ = true;
    width_ = originalWidth;
    height_ = originalHeight;
    (void)rowBytes;
    return true;
}

void Texture2D::destroy() {
    width_ = height_ = storageWidth_ = storageHeight_ = 0;
    rsxOffset_ = 0;
    resident_ = false;
    pixels_.clear();
}

} // namespace pdv
