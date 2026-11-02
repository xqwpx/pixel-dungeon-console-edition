#pragma once
namespace pdv {
class IAudioSystem { public: virtual ~IAudioSystem() = default; virtual bool initialize() = 0; virtual void shutdown() = 0; virtual void playMusic(const char*) = 0; virtual void playSound(const char*) = 0; };
} // namespace pdv
