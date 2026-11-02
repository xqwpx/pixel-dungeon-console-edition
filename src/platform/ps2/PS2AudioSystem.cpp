#include "audio/AudioSystem.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace pdv {
namespace {
constexpr const char* SettingsPath = PD_USERDATA_ROOT "settings.bin";
constexpr unsigned int SettingsMagic = 0x50445653u;
constexpr unsigned int SettingsVersion = 2u;
struct SettingsDataV1 { unsigned int magic, version; unsigned char remixesEnabled; };
struct SettingsData { unsigned int magic, version; unsigned char musicEnabled, soundEnabled, remixesEnabled, language, cameraZoom; };
}
AudioSystem& AudioSystem::instance() { static AudioSystem audio; return audio; }
bool AudioSystem::initialize() { if (initialized_) return true; loadSettings(); Localization::setLanguage(language_); initialized_ = true; return true; }
void AudioSystem::shutdown() { initialized_ = false; currentTrack_.clear(); currentMusic_.clear(); }
void AudioSystem::update() {}
std::string AudioSystem::pathFor(const char* file) const { return std::string(PD_ASSET_ROOT) + (file ? file : ""); }
std::string AudioSystem::musicFileFor(const char* file) const { if (!remixesEnabled_ || !file) return file ? file : ""; if (!std::strcmp(file,"theme.mp3")) return "theme_remix.mp3"; if (!std::strcmp(file,"surface.mp3")) return "surface_remix.mp3"; if (!std::strcmp(file,"game.mp3")) return "game_remix.mp3"; return file; }
void AudioSystem::playMusic(const char* file, int) { if (initialized_ && file) { currentTrack_ = file; currentMusic_ = musicFileFor(file); } }
void AudioSystem::playSound(const char*, float) {}
void AudioSystem::setMusicEnabled(bool enabled) { musicEnabled_ = enabled; saveSettings(); }
void AudioSystem::setSoundEnabled(bool enabled) { soundEnabled_ = enabled; saveSettings(); }
void AudioSystem::setRemixesEnabled(bool enabled) { remixesEnabled_ = enabled; saveSettings(); }
void AudioSystem::setLanguage(Language language) { language_ = language; Localization::setLanguage(language_); saveSettings(); }
void AudioSystem::setCameraZoom(int zoom) { cameraZoom_ = std::max(0, std::min(3, zoom)); saveSettings(); }
void AudioSystem::loadSettings() { FILE* file = std::fopen(SettingsPath, "rb"); if (!file) return; SettingsDataV1 header{}; if (std::fread(&header,1,sizeof(header),file)==sizeof(header) && header.magic==SettingsMagic) { if(header.version==1u) remixesEnabled_=header.remixesEnabled!=0; else if(header.version==SettingsVersion){std::rewind(file);SettingsData data{};if(std::fread(&data,1,sizeof(data),file)==sizeof(data)){musicEnabled_=data.musicEnabled!=0;soundEnabled_=data.soundEnabled!=0;remixesEnabled_=data.remixesEnabled!=0;language_=data.language<=static_cast<unsigned char>(Language::Spanish)?static_cast<Language>(data.language):Language::English;cameraZoom_=std::max(0,std::min(3,static_cast<int>(data.cameraZoom)));}}} std::fclose(file); }
void AudioSystem::saveSettings() const { FILE* file=std::fopen(SettingsPath,"wb");if(!file)return;const SettingsData data{SettingsMagic,SettingsVersion,static_cast<unsigned char>(musicEnabled_),static_cast<unsigned char>(soundEnabled_),static_cast<unsigned char>(remixesEnabled_),static_cast<unsigned char>(language_),static_cast<unsigned char>(cameraZoom_)};std::fwrite(&data,1,sizeof(data),file);std::fclose(file); }
} // namespace pdv
