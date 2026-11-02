#pragma once

#include <cstdint>
#include <vector>

#include "render/TextureRegion.h"

namespace pdv {

class Texture2D;

class PS3Renderer2D {
public:
    bool initialize();
    void shutdown();
    void beginFrame();
    void present();
    void rect(float x, float y, float w, float h, uint32_t rgba, bool additive);
    void texture(const Texture2D& texture, float x, float y, float w, float h,
                 const TextureRegion& region, float alpha, float brightness,
                 bool flipX, uint32_t tint, bool additive);

private:
    static constexpr uint32_t FrameBufferCount = 4;
    // Keep the bring-up vertex layout byte-for-byte equivalent to the
    // PS3DK textured-quad sample: position followed immediately by UV.
    struct Vertex { float position[2]; float uv[2]; };
    void drawQuad(const Texture2D* texture, float x, float y, float w, float h,
                  float u0, float v0, float u1, float v1, uint32_t rgba, bool additive);
    bool setupDisplay();
    void setupShader();
    void bindShaderPrograms();
    void* allocateLocal(uint32_t alignment, uint32_t size);
    void setupDrawEnvironment();
    void setupRenderTarget();
    void bindTexture(const Texture2D* texture);
    void blendPixel(int x, int y, uint32_t rgba, bool additive);
    void uploadSoftwareFrame();
    static void onVBlank(uint32_t head);
    static void onFlip(uint32_t head);
    bool ppuTooFast() const;

    void* hostMemory_ = nullptr;
    void* mainMemory_ = nullptr;
    Vertex* vertices_ = nullptr;
    void* whiteTexture_ = nullptr;
    uint32_t vertexOffset_ = 0;
    uint32_t debugVertexOffset_ = 0;
    uint32_t whiteTextureOffset_ = 0;
    uint32_t whiteTextureWidth_ = 0;
    uint32_t whiteTextureHeight_ = 0;
    void* vertexProgram_ = nullptr;
    void* vertexUcode_ = nullptr;
    void* fragmentProgram_ = nullptr;
    uint32_t fragmentProgramOffset_ = 0;
    uint32_t colorOffset_[FrameBufferCount]{};
    uint32_t* colorAddress_[FrameBufferCount]{};
    uint32_t depthOffset_ = 0;
    uint32_t colorPitch_ = 0;
    uint32_t depthPitch_ = 0;
    uint32_t localHeap_ = 0;
    uint32_t displayWidth_ = 0;
    uint32_t displayHeight_ = 0;
    uint32_t frameBuffer_ = 0;
    volatile uint32_t bufferOnDisplay_ = 0;
    volatile uint32_t bufferFlipped_ = 0;
    volatile uint32_t flipPending_ = 0;
    uint32_t vertexCount_ = 0;
    int positionAttribute_ = 0;
    int texcoordAttribute_ = 8;
    int colorAttribute_ = 3;
    std::vector<uint32_t> softwareFrame_;
    bool initialized_ = false;
};

} // namespace pdv
