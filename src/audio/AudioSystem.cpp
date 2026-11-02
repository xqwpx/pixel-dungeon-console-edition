#include "audio/AudioSystem.h"
#include "assets/AssetPath.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#if defined(PD_PLATFORM_PSP)
#include <pspiofilemgr.h>
#include <vector>
#endif

namespace pdv {

namespace {

#if defined(PD_PLATFORM_PSP)
constexpr const char* SettingsPath = "ms0:/PSP/GAME/PixelDungeon/userdata/settings.bin";
#else
constexpr const char* SettingsPath = PD_USERDATA_ROOT "settings.bin";
#endif
constexpr unsigned int SettingsMagic = 0x50445653u; // PDVS
constexpr unsigned int SettingsVersion = 2u;

struct SettingsDataV1 {
    unsigned int magic;
    unsigned int version;
    unsigned char remixesEnabled;
};

struct SettingsData {
    unsigned int magic;
    unsigned int version;
    unsigned char musicEnabled;
    unsigned char soundEnabled;
    unsigned char remixesEnabled;
    unsigned char language;
    unsigned char cameraZoom;
};

#if defined(PD_PLATFORM_PSP)
bool readPspFile(const std::string& path, std::vector<unsigned char>& data) {
    data.clear();
    const SceUID handle = sceIoOpen(path.c_str(), PSP_O_RDONLY, 0);
    if (handle < 0) return false;

    const SceOff length = sceIoLseek(handle, 0, PSP_SEEK_END);
    if (length <= 0 || length > 8 * 1024 * 1024 || sceIoLseek(handle, 0, PSP_SEEK_SET) < 0) {
        sceIoClose(handle);
        return false;
    }

    data.resize(static_cast<std::size_t>(length));
    std::size_t read = 0;
    while (read < data.size()) {
        const int result = sceIoRead(handle, data.data() + read, data.size() - read);
        if (result <= 0) {
            data.clear();
            sceIoClose(handle);
            return false;
        }
        read += static_cast<std::size_t>(result);
    }
    sceIoClose(handle);
    return true;
}
#endif

} // namespace

AudioSystem& AudioSystem::instance() {
    static AudioSystem audio;
    return audio;
}

void AudioSystem::update() {
    // SDL_mixer owns its output thread on desktop platforms.
}

bool AudioSystem::initialize() {
    if (initialized_) return true;
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) return false;

    Mix_Init(MIX_INIT_MP3);
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) != 0) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return false;
    }

    Mix_AllocateChannels(16);
    loadSettings();
    Localization::setLanguage(language_);
    initialized_ = true;
    return true;
}

void AudioSystem::shutdown() {
    if (!initialized_) return;
    Mix_HaltMusic();
    if (music_) Mix_FreeMusic(music_);
    music_ = nullptr;
    for (auto& entry : sounds_) Mix_FreeChunk(entry.second);
    sounds_.clear();
#if defined(PD_PLATFORM_PSP)
    musicData_.clear();
    soundData_.clear();
#endif
    Mix_CloseAudio();
    Mix_Quit();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    initialized_ = false;
}

std::string AudioSystem::pathFor(const char* file) const {
#if defined(PD_PLATFORM_PSP)
    return pspAssetPath(file ? file : "");
#else
    return std::string(PD_ASSET_ROOT) + file;
#endif
}

std::string AudioSystem::musicFileFor(const char* file) const {
    if (!remixesEnabled_ || file == nullptr) return file ? file : "";

    if (std::strcmp(file, "theme.mp3") == 0) return "theme_remix.mp3";
    if (std::strcmp(file, "surface.mp3") == 0) return "surface_remix.mp3";
    if (std::strcmp(file, "game.mp3") == 0) return "game_remix.mp3";
    return file;
}

void AudioSystem::loadSettings() {
    FILE* file = std::fopen(SettingsPath, "rb");
    if (file == nullptr) return;

    SettingsDataV1 header{};
    const bool hasHeader = std::fread(&header, 1, sizeof(header), file) == sizeof(header) &&
                           header.magic == SettingsMagic;
    if (hasHeader && header.version == 1u) {
        remixesEnabled_ = header.remixesEnabled != 0;
    } else if (hasHeader && header.version == SettingsVersion) {
        std::rewind(file);
        SettingsData data{};
        if (std::fread(&data, 1, sizeof(data), file) == sizeof(data)) {
            musicEnabled_ = data.musicEnabled != 0;
            soundEnabled_ = data.soundEnabled != 0;
            remixesEnabled_ = data.remixesEnabled != 0;
            language_ = data.language <= static_cast<unsigned char>(Language::Spanish)
                ? static_cast<Language>(data.language) : Language::English;
            cameraZoom_ = std::max(0, std::min(3, static_cast<int>(data.cameraZoom)));
        }
    }
    std::fclose(file);
}

void AudioSystem::saveSettings() const {
    const std::string temporaryPath = std::string(SettingsPath) + ".tmp";
    FILE* file = std::fopen(temporaryPath.c_str(), "wb");
    if (file == nullptr) return;

    const SettingsData data{SettingsMagic, SettingsVersion,
                            static_cast<unsigned char>(musicEnabled_ ? 1 : 0),
                            static_cast<unsigned char>(soundEnabled_ ? 1 : 0),
                            static_cast<unsigned char>(remixesEnabled_ ? 1 : 0),
                            static_cast<unsigned char>(language_),
                            static_cast<unsigned char>(cameraZoom_)};
    const bool written = std::fwrite(&data, 1, sizeof(data), file) == sizeof(data) &&
                         std::fflush(file) == 0;
    std::fclose(file);
    if (!written) {
        std::remove(temporaryPath.c_str());
        return;
    }

    std::remove(SettingsPath);
    if (std::rename(temporaryPath.c_str(), SettingsPath) != 0) {
        std::remove(temporaryPath.c_str());
    }
}

void AudioSystem::playMusic(const char* file, int fadeMs) {
    if (!initialized_ || !file) return;
    const std::string musicFile = musicFileFor(file);
    if (currentMusic_ == musicFile && Mix_PlayingMusic()) return;

    Mix_HaltMusic();
    if (music_) Mix_FreeMusic(music_);
#if defined(PD_PLATFORM_PSP)
    musicData_.clear();
    if (readPspFile(pathFor(musicFile.c_str()), musicData_)) {
        SDL_RWops* stream = SDL_RWFromConstMem(musicData_.data(), static_cast<int>(musicData_.size()));
        music_ = stream ? Mix_LoadMUS_RW(stream, 1) : nullptr;
    } else {
        music_ = nullptr;
    }
#else
    music_ = Mix_LoadMUS(pathFor(musicFile.c_str()).c_str());
#endif
    currentTrack_ = file;
    currentMusic_ = musicFile;
    if (music_ && musicEnabled_) Mix_FadeInMusic(music_, -1, fadeMs);
}

void AudioSystem::playSound(const char* file, float volume) {
    if (!initialized_ || !soundEnabled_ || !file) return;
    const std::string key(file);
    Mix_Chunk*& chunk = sounds_[key];
    if (!chunk) {
#if defined(PD_PLATFORM_PSP)
        std::vector<unsigned char>& data = soundData_[key];
        if (readPspFile(pathFor(file), data)) {
            SDL_RWops* stream = SDL_RWFromConstMem(data.data(), static_cast<int>(data.size()));
            chunk = stream ? Mix_LoadWAV_RW(stream, 1) : nullptr;
        }
#else
        chunk = Mix_LoadWAV(pathFor(file).c_str());
#endif
    }
    if (!chunk) return;
    Mix_VolumeChunk(chunk, static_cast<int>(std::max(0.0f, std::min(1.0f, volume)) * MIX_MAX_VOLUME));
    Mix_PlayChannel(-1, chunk, 0);
}

void AudioSystem::setMusicEnabled(bool enabled) {
    musicEnabled_ = enabled;
    saveSettings();
    if (!initialized_) return;
    if (enabled) {
        if (music_ && !Mix_PlayingMusic()) Mix_FadeInMusic(music_, -1, 250);
    } else {
        Mix_HaltMusic();
    }
}

void AudioSystem::setSoundEnabled(bool enabled) {
    soundEnabled_ = enabled;
    saveSettings();
    if (!enabled && initialized_) Mix_HaltChannel(-1);
}

void AudioSystem::setRemixesEnabled(bool enabled) {
    if (remixesEnabled_ == enabled) return;
    remixesEnabled_ = enabled;
    saveSettings();

    if (!currentTrack_.empty()) {
        playMusic(currentTrack_.c_str());
    }
}

void AudioSystem::setLanguage(Language language) {
    if (language_ == language) return;
    language_ = language;
    Localization::setLanguage(language_);
    saveSettings();
}

void AudioSystem::setCameraZoom(int zoom) {
    const int clamped = std::max(0, std::min(3, zoom));
    if (cameraZoom_ == clamped) return;
    cameraZoom_ = clamped;
    saveSettings();
}

} // namespace pdv
