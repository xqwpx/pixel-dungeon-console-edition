#pragma once

#include "core/Localization.h"

#if !defined(PD_PLATFORM_PS3) && !defined(PD_PLATFORM_PS2)
#include <SDL2/SDL_mixer.h>
#endif

#include <string>
#include <unordered_map>
#if defined(PD_PLATFORM_PSP)
#include <vector>
#endif

namespace pdv {

using Language = Localization::Language;

class AudioSystem {
public:
    static AudioSystem& instance();

    bool initialize();
    void shutdown();
    // Advances the native streaming backends.  Desktop SDL_mixer does not
    // require pumping; the PS3 port fills cellAudio's ring from the game loop.
    void update();

    void playMusic(const char* file, int fadeMs = 350);
    void playSound(const char* file, float volume = 1.0f);

    void setMusicEnabled(bool enabled);
    void setSoundEnabled(bool enabled);
    void setRemixesEnabled(bool enabled);
    void setLanguage(Language language);
    void setCameraZoom(int zoom);
    bool musicEnabled() const { return musicEnabled_; }
    bool soundEnabled() const { return soundEnabled_; }
    bool remixesEnabled() const { return remixesEnabled_; }
    Language language() const { return language_; }
    int cameraZoom() const { return cameraZoom_; }

private:
    std::string pathFor(const char* file) const;
    std::string musicFileFor(const char* file) const;
    void loadSettings();
    void saveSettings() const;

    bool initialized_ = false;
    bool musicEnabled_ = true;
    bool soundEnabled_ = true;
    bool remixesEnabled_ = false;
    Language language_ = Language::English;
    int cameraZoom_ = 3;
    std::string currentTrack_;
    std::string currentMusic_;
#if !defined(PD_PLATFORM_PS3) && !defined(PD_PLATFORM_PS2)
    Mix_Music* music_ = nullptr;
    std::unordered_map<std::string, Mix_Chunk*> sounds_;
#if defined(PD_PLATFORM_PSP)
    // SDL_mixer may keep an RW stream open for MP3 music, so the backing
    // storage must remain valid for the lifetime of the decoded asset.
    std::vector<unsigned char> musicData_;
    std::unordered_map<std::string, std::vector<unsigned char>> soundData_;
#endif
#endif
};

} // namespace pdv
