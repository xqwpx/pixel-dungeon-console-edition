#include "platform/ps3/PS3Renderer2D.h"

#include "assets/Texture2D.h"
#include "platform/ps3/PS3TextureAllocator.h"
#include "render/Renderer2D.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>

#include <cell/gcm.h>
#include <cell/sysutil.h>
#include <rsx/rsx.h>
#include <sysutil/video.h>

#include "Renderer2D_vpo.h"
#include "Renderer2D_fpo.h"

namespace pdv {
namespace {
constexpr uint32_t CommandBufferSize = 1024 * 1024;
constexpr uint32_t HostMemorySize = 32 * 1024 * 1024;
constexpr uint32_t VertexCapacity = 65536;
constexpr uint8_t GcmLabelIndex = 255;
constexpr uint8_t PreparedBufferLabel = 0x41;
constexpr uint8_t BufferStatusLabel = 0x42;
constexpr uint32_t BufferIdle = 0;
constexpr uint32_t BufferBusy = 1;
// Temporary bring-up switch.  Rendering must first prove the unmodified
// PS3DK clear/flip path before any Cg vertex commands are admitted.
// Diagnostic: keep the GCM path to one clear plus the verified PS3DK
// PrepareFlip pipeline.  If this is not a solid colour, shaders and vertex
// data are categorically not involved in the corruption.
constexpr bool ClearOnlyBringUp = false;
constexpr bool SolidColorBringUp = false;
constexpr bool SingleQuadBringUp = false;
// Kept on until the first native quad is observed after a renderer change.
// This texture is generated locally in ARGB and bypasses PNG decoding.
constexpr bool DrawKnownGoodQuad = false;
PS3Renderer2D* activeRenderer = nullptr;
uint32_t textureLocalHeap = 0;

uint32_t alignUp(uint32_t value, uint32_t alignment) {
    return (value + alignment - 1u) & ~(alignment - 1u);
}

float channel(uint32_t rgba, int shift) {
    return static_cast<float>((rgba >> shift) & 0xffu) / 255.0f;
}

void waitRsxIdle(CellGcmContextData* ctx) {
    uint32_t label = 1;
    cellGcmSetWriteBackEndLabel(ctx, GcmLabelIndex, label);
    cellGcmSetWaitLabel(ctx, GcmLabelIndex, label);
    ++label;
    cellGcmSetWriteBackEndLabel(ctx, GcmLabelIndex, label);
    cellGcmFlush(ctx);
    while (*reinterpret_cast<volatile uint32_t*>(cellGcmGetLabelAddress(GcmLabelIndex)) != label) {
        usleep(30);
    }
}
}

void ps3ConfigureTextureAllocator(uint32_t localAddress, uint32_t localSize, uint32_t firstFreeAddress) {
    (void)localAddress;
    (void)localSize;
    textureLocalHeap = firstFreeAddress;
}

bool ps3UploadTexture(const void* data, uint32_t size, uint32_t* offset) {
    if (data == nullptr || offset == nullptr || textureLocalHeap == 0) return false;
    textureLocalHeap = alignUp(textureLocalHeap, 128);
    void* destination = reinterpret_cast<void*>(static_cast<uintptr_t>(textureLocalHeap));
    textureLocalHeap += alignUp(size, 1024);
    std::memcpy(destination, data, size);
    return cellGcmAddressToOffset(destination, offset) == 0;
}

bool PS3Renderer2D::initialize() {
    hostMemory_ = memalign(1024 * 1024, HostMemorySize);
    if (hostMemory_ == nullptr) {
        std::printf("PS3Renderer: host command memory allocation failed\n");
        return false;
    }
    const int initResult = cellGcmInit(CommandBufferSize, HostMemorySize, hostMemory_);
    if (initResult != 0) {
        std::printf("PS3Renderer: cellGcmInit failed: 0x%08x\n", initResult);
        shutdown();
        return false;
    }
    CellGcmConfig gcmConfig{};
    cellGcmGetConfiguration(&gcmConfig);
    localHeap_ = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(gcmConfig.localAddress));
    if (!setupDisplay()) {
        std::printf("PS3Renderer: display setup failed\n");
        shutdown();
        return false;
    }
    if (!ClearOnlyBringUp) setupShader();
    // Dynamic vertex data is local too. This avoids the limited and
    // cache-sensitive RSX main-memory mapping path altogether.
    // The RSX consumes vertices asynchronously.  Keep one complete dynamic
    // vertex arena per display buffer, so the PPU never overwrites vertices
    // that are still being read by an earlier frame.
    vertices_ = static_cast<Vertex*>(allocateLocal(128, sizeof(Vertex) * VertexCapacity * FrameBufferCount));
    if (cellGcmAddressToOffset(vertices_, &vertexOffset_) != 0) {
        std::printf("PS3Renderer: local vertex buffer allocation failed\n");
        shutdown();
        return false;
    }
    // One immutable quad for bring-up.  It is deliberately not part of the
    // streaming arena: it proves the RSX state independently of CPU/RSX
    // ownership of dynamically-written vertices.
    auto* debugVertices = static_cast<Vertex*>(allocateLocal(128, 4 * sizeof(Vertex)));
    debugVertices[0] = {{-0.4f,  0.32f}, {0.0f, 0.0f}};
    debugVertices[1] = {{ 0.4f,  0.32f}, {1.0f, 0.0f}};
    debugVertices[2] = {{-0.4f, -0.32f}, {0.0f, 1.0f}};
    debugVertices[3] = {{ 0.4f, -0.32f}, {1.0f, 1.0f}};
    if (cellGcmAddressToOffset(debugVertices, &debugVertexOffset_) != 0) {
        std::printf("PS3Renderer: debug vertex allocation failed\\n");
        shutdown();
        return false;
    }
    // Keep the fallback texture identical in shape to PS3DK's verified quad
    // sample. A 1x1 texture is accepted by some RSX implementations but not
    // reliably sampled by the hardware path.
    whiteTextureWidth_ = 64;
    whiteTextureHeight_ = 64;
    auto* white = static_cast<uint32_t*>(allocateLocal(128, whiteTextureWidth_ * whiteTextureHeight_ * sizeof(uint32_t)));
    for (uint32_t y = 0; y < whiteTextureHeight_; ++y) {
        for (uint32_t x = 0; x < whiteTextureWidth_; ++x) {
            white[y * whiteTextureWidth_ + x] = ((x >> 3) ^ (y >> 3)) & 1u ? 0xff20b0c0u : 0xffe0a040u;
        }
    }
    if (cellGcmAddressToOffset(white, &whiteTextureOffset_) != 0) {
        std::printf("PS3Renderer: white texture allocation failed\n");
        shutdown();
        return false;
    }
    ps3ConfigureTextureAllocator(0, 0, localHeap_);
    activeRenderer = this;
    initialized_ = true;
    setupDrawEnvironment();
    if (vertexProgram_ != nullptr) {
        // The Cg literal pool is immutable.  PS3DK's working sample uploads
        // it once before the first draw; emitting it for every quad eventually
        // fills the FIFO and stalls the RSX command processor.
        cellGcmCgUploadInternalConsts(CELL_GCM_CURRENT, reinterpret_cast<CGprogram>(vertexProgram_));
    }
    setupRenderTarget();
    cellGcmSetWriteCommandLabel(CELL_GCM_CURRENT, BufferStatusLabel + frameBuffer_, BufferBusy);
    std::printf("PS3Renderer: native cellGcm ready (%ux%u), quad-frame-pipeline r7\n", displayWidth_, displayHeight_);
    return true;
}

bool PS3Renderer2D::setupDisplay() {
    videoState state{};
    videoResolution resolution{};
    const int stateResult = videoGetState(0, 0, &state);
    const int resolutionResult = stateResult == 0 ? videoGetResolution(state.displayMode.resolution, &resolution) : -1;
    if (stateResult != 0 || state.state != 0 || resolutionResult != 0) {
        std::printf("PS3Renderer: video state failed (%d, state=%d, resolution=%d)\n", stateResult, state.state, resolutionResult);
        return false;
    }
    displayWidth_ = resolution.width;
    displayHeight_ = resolution.height;
    colorPitch_ = displayWidth_ * 4;
    // ZETA_Z16 has a 16-bit depth element.  Clear-only happens to tolerate a
    // wider pitch, but the RSX rasterizer requires this exact layout once
    // primitives are emitted.
    depthPitch_ = displayWidth_ * 2;
    videoConfiguration config{};
    config.resolution = state.displayMode.resolution;
    config.format = VIDEO_BUFFER_FORMAT_XRGB;
    config.pitch = colorPitch_;
    config.aspect = state.displayMode.aspect;
    waitRsxIdle(CELL_GCM_CURRENT);
    const int configureResult = videoConfigure(0, &config, nullptr, 0);
    if (configureResult != 0) {
        std::printf("PS3Renderer: videoConfigure failed: 0x%08x\n", configureResult);
        return false;
    }
    cellGcmSetFlipMode(GCM_FLIP_HSYNC);

    const uint32_t colorSize = colorPitch_ * displayHeight_;
    for (uint32_t i = 0; i < FrameBufferCount; ++i) {
        void* address = allocateLocal(64, colorSize);
        if (address == nullptr) return false;
        colorAddress_[i] = static_cast<uint32_t*>(address);
        // The software compositor only writes its centred integer-scale
        // viewport.  Initialise the surrounding letterbox once per buffer.
        std::memset(address, 0, colorSize);
        const int offsetResult = cellGcmAddressToOffset(address, &colorOffset_[i]);
        const int bufferResult = offsetResult == 0 ? cellGcmSetDisplayBuffer(static_cast<uint8_t>(i), colorOffset_[i], colorPitch_, displayWidth_, displayHeight_) : -1;
        if (offsetResult != 0 || bufferResult != 0) {
            std::printf("PS3Renderer: display buffer %u failed (%d, %d)\n", i, offsetResult, bufferResult);
            return false;
        }
    }
    void* depth = allocateLocal(64, depthPitch_ * displayHeight_);
    if (depth == nullptr || cellGcmAddressToOffset(depth, &depthOffset_) != 0) return false;
    for (uint32_t i = 0; i < FrameBufferCount; ++i) {
        *reinterpret_cast<volatile uint32_t*>(cellGcmGetLabelAddress(BufferStatusLabel + i)) = BufferIdle;
    }
    *reinterpret_cast<volatile uint32_t*>(cellGcmGetLabelAddress(PreparedBufferLabel)) = 0;
    *reinterpret_cast<volatile uint32_t*>(cellGcmGetLabelAddress(BufferStatusLabel)) = BufferBusy;
    frameBuffer_ = 1;
    cellGcmSetVBlankHandler(onVBlank);
    cellGcmSetFlipHandler(onFlip);
    return true;
}

void PS3Renderer2D::setupShader() {
    CGprogram vp = reinterpret_cast<CGprogram>(const_cast<unsigned char*>(Renderer2D_vpo));
    CGprogram fp = reinterpret_cast<CGprogram>(const_cast<unsigned char*>(Renderer2D_fpo));
    cellGcmCgInitProgram(vp);
    cellGcmCgInitProgram(fp);
    void* vpUcode = nullptr;
    void* fpBlob = nullptr;
    uint32_t unused = 0;
    cellGcmCgGetUCode(vp, &vpUcode, &unused);
    cellGcmCgGetUCode(fp, &fpBlob, &unused);
    void* fpUcode = allocateLocal(64, unused);
    if (fpUcode == nullptr) return;
    std::memcpy(fpUcode, fpBlob, unused);
    uint32_t fpOffset = 0;
    cellGcmAddressToOffset(fpUcode, &fpOffset);
    vertexProgram_ = vp;
    vertexUcode_ = vpUcode;
    fragmentProgram_ = fp;
    fragmentProgramOffset_ = fpOffset;
    CGparameter position = cellGcmCgGetNamedParameter(vp, "in_position");
    CGparameter texcoord = cellGcmCgGetNamedParameter(vp, "in_texcoord");
    if (position != nullptr) positionAttribute_ = static_cast<int>(cellGcmCgGetParameterResource(vp, position)) - CG_ATTR0;
    if (texcoord != nullptr) texcoordAttribute_ = static_cast<int>(cellGcmCgGetParameterResource(vp, texcoord)) - CG_ATTR0;
}

void* PS3Renderer2D::allocateLocal(uint32_t alignment, uint32_t size) {
    localHeap_ = alignUp(localHeap_, alignment);
    const uint32_t address = localHeap_;
    // PS3DK's local allocator advances in 1 KiB blocks; preserve that
    // convention for all render resources so they never overlap.
    localHeap_ += alignUp(size, 1024);
    return reinterpret_cast<void*>(static_cast<uintptr_t>(address));
}

void PS3Renderer2D::bindShaderPrograms() {
    if (vertexProgram_ == nullptr || fragmentProgram_ == nullptr) return;
    CellGcmContextData* ctx = CELL_GCM_CURRENT;
    CGprogram vp = reinterpret_cast<CGprogram>(vertexProgram_);
    cellGcmSetVertexProgram(ctx, vp, vertexUcode_);
    cellGcmSetFragmentProgram(ctx, reinterpret_cast<CGprogram>(fragmentProgram_), fragmentProgramOffset_);
}

void PS3Renderer2D::setupDrawEnvironment() {
    CellGcmContextData* ctx = CELL_GCM_CURRENT;
    cellGcmSetColorMask(ctx, GCM_COLOR_MASK_R | GCM_COLOR_MASK_G | GCM_COLOR_MASK_B | GCM_COLOR_MASK_A);
    cellGcmSetColorMaskMrt(ctx, 0);
    float scale[4] = {displayWidth_ * .5f, -displayHeight_ * .5f, .5f, 0.f};
    float offset[4] = {displayWidth_ * .5f, displayHeight_ * .5f, .5f, 0.f};
    cellGcmSetViewport(ctx, 0, 0, displayWidth_, displayHeight_, 0.f, 1.f, scale, offset);
    rsxSetScissor(ctx, 0, 0, displayWidth_, displayHeight_);
    cellGcmSetDepthTestEnable(ctx, GCM_FALSE);
    cellGcmSetShadeModel(ctx, GCM_SHADE_MODEL_SMOOTH);
    cellGcmSetCullFaceEnable(ctx, GCM_FALSE);
    cellGcmSetBlendEnable(ctx, GCM_TRUE);
    cellGcmSetBlendEquation(ctx, CELL_GCM_FUNC_ADD, CELL_GCM_FUNC_ADD);
}

void PS3Renderer2D::setupRenderTarget() {
    CellGcmContextData* ctx = CELL_GCM_CURRENT;
    CellGcmSurface surface{};
    surface.colorFormat = GCM_SURFACE_X8R8G8B8;
    surface.colorTarget = GCM_SURFACE_TARGET_0;
    surface.colorLocation[0] = GCM_LOCATION_RSX;
    surface.colorOffset[0] = colorOffset_[frameBuffer_];
    surface.colorPitch[0] = colorPitch_;
    surface.colorLocation[1] = GCM_LOCATION_RSX;
    surface.colorLocation[2] = GCM_LOCATION_RSX;
    surface.colorLocation[3] = GCM_LOCATION_RSX;
    surface.colorPitch[1] = 64;
    surface.colorPitch[2] = 64;
    surface.colorPitch[3] = 64;
    surface.depthFormat = GCM_SURFACE_ZETA_Z16;
    surface.depthLocation = GCM_LOCATION_RSX;
    surface.depthOffset = depthOffset_;
    surface.depthPitch = depthPitch_;
    surface.type = GCM_SURFACE_TYPE_LINEAR;
    surface.antiAlias = GCM_SURFACE_CENTER_1;
    surface.width = displayWidth_;
    surface.height = displayHeight_;
    surface.x = 0;
    surface.y = 0;
    cellGcmSetSurface(ctx, &surface);
}

void PS3Renderer2D::shutdown() {
    if (initialized_) cellGcmFinish(CELL_GCM_CURRENT, 0);
    std::free(hostMemory_);
    vertices_ = nullptr;
    hostMemory_ = nullptr;
    initialized_ = false;
    if (activeRenderer == this) activeRenderer = nullptr;
}

void PS3Renderer2D::beginFrame() {
    if (!initialized_) return;
    vertexCount_ = 0;
    softwareFrame_.assign(Renderer2D::LogicalWidth * Renderer2D::LogicalHeight, 0xff203050u);
}

void PS3Renderer2D::present() {
    if (!initialized_) return;
    CellGcmContextData* ctx = CELL_GCM_CURRENT;
    uploadSoftwareFrame();
    int32_t queueId = cellGcmSetPrepareFlip(ctx, static_cast<uint8_t>(frameBuffer_));
    while (queueId < 0) {
        usleep(1000);
        queueId = cellGcmSetPrepareFlip(ctx, static_cast<uint8_t>(frameBuffer_));
    }
    cellGcmSetWriteBackEndLabel(ctx, PreparedBufferLabel, (frameBuffer_ << 8) | static_cast<uint32_t>(queueId));
    cellGcmFlush(ctx);
    while (ppuTooFast()) usleep(3000);
    // Match the reference pipeline: state for the next frame is emitted
    // before selecting its render target and acquiring that buffer label.
    setupDrawEnvironment();
    frameBuffer_ = (frameBuffer_ + 1u) % FrameBufferCount;
    cellGcmSetWaitLabel(ctx, BufferStatusLabel + frameBuffer_, BufferIdle);
    cellGcmSetWriteCommandLabel(ctx, BufferStatusLabel + frameBuffer_, BufferBusy);
    setupRenderTarget();
}

bool PS3Renderer2D::ppuTooFast() const {
    const uint32_t gpu = *reinterpret_cast<volatile uint32_t*>(cellGcmGetLabelAddress(PreparedBufferLabel)) >> 8;
    return ((frameBuffer_ + FrameBufferCount - gpu) % FrameBufferCount) > 1;
}

void PS3Renderer2D::onFlip(uint32_t) {
    if (activeRenderer == nullptr) return;
    PS3Renderer2D& renderer = *activeRenderer;
    for (uint32_t i = renderer.bufferOnDisplay_; i != renderer.bufferFlipped_; i = (i + 1u) % FrameBufferCount) {
        *reinterpret_cast<volatile uint32_t*>(cellGcmGetLabelAddress(BufferStatusLabel + i)) = BufferIdle;
    }
    renderer.bufferOnDisplay_ = renderer.bufferFlipped_;
    renderer.flipPending_ = 0;
}

void PS3Renderer2D::onVBlank(uint32_t) {
    if (activeRenderer == nullptr) return;
    PS3Renderer2D& renderer = *activeRenderer;
    const uint32_t data = *reinterpret_cast<volatile uint32_t*>(cellGcmGetLabelAddress(PreparedBufferLabel));
    const uint32_t buffer = data >> 8;
    const uint32_t queueId = data & 0x7u;
    if (renderer.flipPending_ == 0 && buffer != renderer.bufferOnDisplay_ && cellGcmSetFlipImmediate(static_cast<uint8_t>(queueId)) == 0) {
        renderer.flipPending_ = 1;
        renderer.bufferFlipped_ = buffer;
    }
}

void PS3Renderer2D::bindTexture(const Texture2D* texture) {
    const int width = texture != nullptr && texture->valid() ? texture->storageWidth() : static_cast<int>(whiteTextureWidth_);
    const int height = texture != nullptr && texture->valid() ? texture->storageHeight() : static_cast<int>(whiteTextureHeight_);
    const uint32_t offset = texture != nullptr && texture->valid() ? texture->nativeOffset() : whiteTextureOffset_;
    CellGcmTexture value{};
    value.format = CELL_GCM_TEXTURE_A8R8G8B8 | CELL_GCM_TEXTURE_LN | CELL_GCM_TEXTURE_NR;
    value.mipmap = 1; value.dimension = CELL_GCM_TEXTURE_DIMENSION_2; value.depth = 1;
    value.width = width; value.height = height; value.location = CELL_GCM_LOCATION_LOCAL;
    value.pitch = width * 4; value.offset = offset;
    value.remap = (CELL_GCM_TEXTURE_REMAP_REMAP << 8) | (CELL_GCM_TEXTURE_REMAP_REMAP << 10) |
                  (CELL_GCM_TEXTURE_REMAP_REMAP << 12) | (CELL_GCM_TEXTURE_REMAP_REMAP << 14) |
                  (CELL_GCM_TEXTURE_REMAP_FROM_A << 0) | (CELL_GCM_TEXTURE_REMAP_FROM_R << 2) |
                  (CELL_GCM_TEXTURE_REMAP_FROM_G << 4) | (CELL_GCM_TEXTURE_REMAP_FROM_B << 6);
    // Texture uploads are made by the PPU into RSX-local memory.  Invalidate
    // the sampler cache before rebinding, otherwise the RSX may retain data
    // from an earlier atlas at the same cache lines.
    cellGcmSetInvalidateTextureCache(CELL_GCM_CURRENT, CELL_GCM_INVALIDATE_TEXTURE);
    cellGcmSetTexture(CELL_GCM_CURRENT, 0, &value);
    cellGcmSetTextureControl(CELL_GCM_CURRENT, 0, CELL_GCM_TRUE, 0, 0, CELL_GCM_TEXTURE_MAX_ANISO_1);
    cellGcmSetTextureFilter(CELL_GCM_CURRENT, 0, 0, CELL_GCM_TEXTURE_NEAREST, CELL_GCM_TEXTURE_NEAREST, CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX);
    cellGcmSetTextureAddress(CELL_GCM_CURRENT, 0, CELL_GCM_TEXTURE_CLAMP_TO_EDGE, CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
                             CELL_GCM_TEXTURE_CLAMP_TO_EDGE, CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL, CELL_GCM_TEXTURE_ZFUNC_LESS, 0);
}

void PS3Renderer2D::drawQuad(const Texture2D* texture, float x, float y, float w, float h,
                              float u0, float v0, float u1, float v1, uint32_t rgba, bool additive) {
    if (ClearOnlyBringUp || !initialized_ || vertexCount_ + 4 > VertexCapacity) return;
    if (SingleQuadBringUp && vertexCount_ != 0) return;
    const float left = x * 2.f / Renderer2D::LogicalWidth - 1.f;
    const float right = (x + w) * 2.f / Renderer2D::LogicalWidth - 1.f;
    const float top = 1.f - y * 2.f / Renderer2D::LogicalHeight;
    const float bottom = 1.f - (y + h) * 2.f / Renderer2D::LogicalHeight;
    CellGcmContextData* ctx = CELL_GCM_CURRENT;
    cellGcmSetVertexProgram(ctx, reinterpret_cast<CGprogram>(vertexProgram_), vertexUcode_);
    cellGcmSetBlendFunc(ctx, additive ? CELL_GCM_SRC_ALPHA : CELL_GCM_SRC_ALPHA,
                         additive ? CELL_GCM_ONE : CELL_GCM_ONE_MINUS_SRC_ALPHA,
                         CELL_GCM_ONE, CELL_GCM_ONE_MINUS_SRC_ALPHA);
    const uint32_t frameVertexBase = frameBuffer_ * VertexCapacity;
    Vertex* v = vertices_ + frameVertexBase + vertexCount_;
    const float positions[4][2] = {{left, top}, {right, top}, {left, bottom}, {right, bottom}};
    const float uvs[4][2] = {{u0, v0}, {u1, v0}, {u0, v1}, {u1, v1}};
    for (int i = 0; i < 4; ++i) {
        std::memcpy(v[i].position, positions[i], sizeof(v[i].position));
        std::memcpy(v[i].uv, uvs[i], sizeof(v[i].uv));
    }
    const uint32_t base = vertexOffset_ + (frameVertexBase + vertexCount_) * sizeof(Vertex);
    cellGcmSetVertexDataArray(ctx, positionAttribute_, 0, sizeof(Vertex), 2, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, base);
    cellGcmSetVertexDataArray(ctx, texcoordAttribute_, 0, sizeof(Vertex), 2, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, base + 8);
    cellGcmSetFragmentProgram(ctx, reinterpret_cast<CGprogram>(fragmentProgram_), fragmentProgramOffset_);
    bindTexture(texture);
    rsxDrawVertexArray(ctx, GCM_TYPE_TRIANGLE_STRIP, 0, 4);
    vertexCount_ += 4;
}

void PS3Renderer2D::rect(float x, float y, float w, float h, uint32_t rgba, bool additive) {
    const int x0 = std::max(0, static_cast<int>(std::floor(x)));
    const int y0 = std::max(0, static_cast<int>(std::floor(y)));
    const int x1 = std::min(Renderer2D::LogicalWidth, static_cast<int>(std::ceil(x + w)));
    const int y1 = std::min(Renderer2D::LogicalHeight, static_cast<int>(std::ceil(y + h)));
    for (int py = y0; py < y1; ++py) for (int px = x0; px < x1; ++px) blendPixel(px, py, rgba, additive);
}

void PS3Renderer2D::texture(const Texture2D& texture, float x, float y, float w, float h,
                            const TextureRegion& region, float alpha, float brightness,
                            bool flipX, uint32_t tint, bool additive) {
    if (!texture.valid() || texture.rgbaPixels() == nullptr || w <= 0.f || h <= 0.f) return;
    const float b = std::max(0.f, std::min(1.f, brightness));
    const float a = std::max(0.f, std::min(1.f, alpha));
    // The dungeon camera supplies fractional positions.  Sampling a pixel
    // atlas against those fractions causes per-frame rescaling and softens
    // tiles; snap the destination rectangle before nearest-neighbour lookup.
    const int dstX = static_cast<int>(std::lround(x));
    const int dstY = static_cast<int>(std::lround(y));
    const int dstW = std::max(1, static_cast<int>(std::lround(w)));
    const int dstH = std::max(1, static_cast<int>(std::lround(h)));
    const int x0 = std::max(0, dstX), y0 = std::max(0, dstY);
    const int x1 = std::min(Renderer2D::LogicalWidth, dstX + dstW), y1 = std::min(Renderer2D::LogicalHeight, dstY + dstH);
    const uint8_t* pixels = texture.rgbaPixels();
    const uint32_t tintR = static_cast<uint32_t>(channel(tint, 24) * b * 255.f);
    const uint32_t tintG = static_cast<uint32_t>(channel(tint, 16) * b * 255.f);
    const uint32_t tintB = static_cast<uint32_t>(channel(tint, 8) * b * 255.f);
    const uint32_t alphaByte = static_cast<uint32_t>(a * 255.f);
    for (int py = y0; py < y1; ++py) for (int px = x0; px < x1; ++px) {
        float fx = static_cast<float>(px - dstX) / dstW;
        const float fy = static_cast<float>(py - dstY) / dstH;
        if (flipX) fx = 1.f - fx;
        const int sx = std::max(0, std::min(texture.width() - 1, static_cast<int>(region.x + fx * region.w)));
        const int sy = std::max(0, std::min(texture.height() - 1, static_cast<int>(region.y + fy * region.h)));
        const uint8_t* p = pixels + (sy * texture.storageWidth() + sx) * 4;
        const uint32_t rgba = ((static_cast<uint32_t>(p[0]) * tintR / 255u) << 24) |
                              ((static_cast<uint32_t>(p[1]) * tintG / 255u) << 16) |
                              ((static_cast<uint32_t>(p[2]) * tintB / 255u) << 8) |
                              (static_cast<uint32_t>(p[3]) * alphaByte / 255u);
        blendPixel(px, py, rgba, additive);
    }
}

void PS3Renderer2D::blendPixel(int x, int y, uint32_t rgba, bool additive) {
    if (x < 0 || y < 0 || x >= Renderer2D::LogicalWidth || y >= Renderer2D::LogicalHeight) return;
    const uint32_t srcA = rgba & 0xffu;
    if (srcA == 0) return;
    const uint32_t srcR = (rgba >> 24) & 0xffu, srcG = (rgba >> 16) & 0xffu, srcB = (rgba >> 8) & 0xffu;
    uint32_t& dst = softwareFrame_[y * Renderer2D::LogicalWidth + x];
    if (!additive && srcA == 255) { dst = 0xff000000u | (rgba >> 8); return; }
    const uint32_t dstR = (dst >> 16) & 0xffu, dstG = (dst >> 8) & 0xffu, dstB = dst & 0xffu;
    const uint32_t r = additive ? std::min(255u, dstR + srcR * srcA / 255u) : (srcR * srcA + dstR * (255u - srcA)) / 255u;
    const uint32_t g = additive ? std::min(255u, dstG + srcG * srcA / 255u) : (srcG * srcA + dstG * (255u - srcA)) / 255u;
    const uint32_t b = additive ? std::min(255u, dstB + srcB * srcA / 255u) : (srcB * srcA + dstB * (255u - srcA)) / 255u;
    dst = 0xff000000u | (r << 16) | (g << 8) | b;
}

void PS3Renderer2D::uploadSoftwareFrame() {
    uint32_t* dst = colorAddress_[frameBuffer_];
    if (dst == nullptr) return;
    const uint32_t stride = colorPitch_ / 4;
    // 480:272 is not an integer divisor of 720p.  Fit it to 1270x720
    // (rather than distort it to 1280x720): this preserves the original
    // aspect ratio, fills practically the whole screen, and keeps gameplay
    // coordinates in the existing 480x272 virtual canvas.
    const uint32_t outHeight = displayHeight_;
    const uint32_t outWidth = std::min(displayWidth_,
        static_cast<uint32_t>(Renderer2D::LogicalWidth) * outHeight / Renderer2D::LogicalHeight);
    const uint32_t offsetX = (displayWidth_ - outWidth) / 2;
    const uint32_t offsetY = (displayHeight_ - outHeight) / 2;
    for (uint32_t y = 0; y < outHeight; ++y) {
        const uint32_t sy = y * Renderer2D::LogicalHeight / outHeight;
        uint32_t* row = dst + (offsetY + y) * stride + offsetX;
        for (uint32_t x = 0; x < outWidth; ++x) {
            row[x] = softwareFrame_[sy * Renderer2D::LogicalWidth + x * Renderer2D::LogicalWidth / outWidth];
        }
    }
}

} // namespace pdv
