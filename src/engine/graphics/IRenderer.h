#pragma once
namespace pdv {
class IRenderer { public: virtual ~IRenderer() = default; virtual bool initialize() = 0; virtual void shutdown() = 0; virtual void beginFrame() = 0; virtual void endFrame() = 0; };
} // namespace pdv
