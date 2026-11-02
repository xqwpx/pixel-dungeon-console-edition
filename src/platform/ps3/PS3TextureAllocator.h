#pragma once

#include <cstdint>

namespace pdv {
void ps3ConfigureTextureAllocator(uint32_t localAddress, uint32_t localSize, uint32_t firstFreeAddress);
bool ps3UploadTexture(const void* data, uint32_t size, uint32_t* offset);
}
