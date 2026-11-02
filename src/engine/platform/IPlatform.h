#pragma once

namespace pdv {
class IInputBackend;
class IRenderer;
class IAudioSystem;
class IFileSystem;
class ISaveStorage;
class ITimeSource;
class ILogger;

struct PlatformCapabilities {
    bool touch = false, mouse = false, keyboard = false, secondScreen = false;
    bool analogLeft = false, analogRight = false, vibration = false;
    int preferredWidth = 480, preferredHeight = 272;
};

class IPlatform {
public:
    virtual ~IPlatform() = default;
    virtual const char* id() const = 0;
    virtual PlatformCapabilities capabilities() const = 0;
};
} // namespace pdv
