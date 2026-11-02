#include "audio/AudioSystem.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <vector>

#include <cell/audio.h>
#include <mad.h>

namespace pdv {
namespace {
constexpr const char* SettingsPath = PD_USERDATA_ROOT "settings.bin";
constexpr unsigned int SettingsMagic = 0x50445653u;
constexpr unsigned int SettingsVersion = 2u;
constexpr unsigned int OutputRate = 48000u;
constexpr unsigned int BlockSamples = CELL_AUDIO_BLOCK_SAMPLES;
constexpr unsigned int MaxVoices = 12u;
constexpr size_t InputBytes = 32768u;

struct SettingsDataV1 { unsigned int magic, version; unsigned char remixesEnabled; };
struct SettingsData { unsigned int magic, version; unsigned char musicEnabled, soundEnabled, remixesEnabled, language, cameraZoom; };

static float madToFloat(mad_fixed_t value) {
    if (value >= MAD_F_ONE) return 1.0f;
    if (value <= -MAD_F_ONE) return -1.0f;
    return static_cast<float>(value) / static_cast<float>(MAD_F_ONE);
}

// Decodes one MP3 at a time from a small input window.  Keeping the music
// streaming is important on PS3: the remixes are several megabytes compressed
// and much larger as PCM.
struct MusicStream {
    FILE* file = nullptr;
    mad_stream stream{};
    mad_frame frame{};
    mad_synth synth{};
    std::array<unsigned char, InputBytes + MAD_BUFFER_GUARD> input{};
    std::vector<float> decoded;
    size_t sample = 0;
    double phase = 0.0;
    unsigned int rate = OutputRate;
    bool eof = false;
    bool decoderInitialized = false;
    bool looping = true;

    void close() {
        if (file) std::fclose(file);
        file = nullptr;
        if (decoderInitialized) {
            mad_synth_finish(&synth); mad_frame_finish(&frame); mad_stream_finish(&stream);
            decoderInitialized = false;
        }
        decoded.clear(); sample = 0; phase = 0.0; eof = false;
    }

    bool open(const char* path, bool repeat = true) {
        close();
        looping = repeat;
        file = std::fopen(path, "rb");
        if (!file) return false;
        mad_stream_init(&stream); mad_frame_init(&frame); mad_synth_init(&synth); decoderInitialized = true;
        return refill();
    }

    bool refill() {
        if (!file) return false;
        size_t keep = 0;
        if (stream.next_frame && stream.bufend && stream.next_frame < stream.bufend) {
            keep = static_cast<size_t>(stream.bufend - stream.next_frame);
            std::memmove(input.data(), stream.next_frame, keep);
        }
        const size_t read = std::fread(input.data() + keep, 1, InputBytes - keep, file);
        eof = read == 0;
        if (eof) std::memset(input.data() + keep, 0, MAD_BUFFER_GUARD);
        mad_stream_buffer(&stream, input.data(), keep + read + (eof ? MAD_BUFFER_GUARD : 0));
        stream.error = MAD_ERROR_NONE;
        return read != 0 || keep != 0;
    }

    bool decodeFrame() {
        // libmad may report a recoverable sync error without consuming a
        // complete frame at a truncated MP3 tail.  Audio must never hold the
        // game loop hostage in that case.
        for (unsigned int attempts = 0; attempts < 64u; ++attempts) {
            if (mad_frame_decode(&frame, &stream) == 0) {
                mad_synth_frame(&synth, &frame);
                rate = synth.pcm.samplerate ? synth.pcm.samplerate : OutputRate;
                decoded.resize(static_cast<size_t>(synth.pcm.length) * 2u);
                for (unsigned int i = 0; i < synth.pcm.length; ++i) {
                    decoded[i * 2] = madToFloat(synth.pcm.samples[0][i]);
                    decoded[i * 2 + 1] = madToFloat(synth.pcm.samples[synth.pcm.channels == 1 ? 0 : 1][i]);
                }
                sample = 0;
                return true;
            }
            if (MAD_RECOVERABLE(stream.error)) continue;
            if (stream.error == MAD_ERROR_BUFLEN && refill()) continue;
            return false;
        }
        return false;
    }

    bool restart() {
        if (!file) return false;
        std::rewind(file);
        mad_synth_finish(&synth); mad_frame_finish(&frame); mad_stream_finish(&stream);
        mad_stream_init(&stream); mad_frame_init(&frame); mad_synth_init(&synth);
        decoderInitialized = true;
        decoded.clear(); sample = 0; phase = 0.0; eof = false;
        return refill();
    }

    bool next(float& left, float& right) {
        for (;;) {
            if (sample < decoded.size() / 2u) {
                left = decoded[sample * 2]; right = decoded[sample * 2 + 1];
                phase += static_cast<double>(rate);
                while (phase >= OutputRate) { phase -= OutputRate; ++sample; }
                return true;
            }
            if (decodeFrame()) continue;
            if (!looping || !restart()) return false;
        }
    }
};

struct Voice { MusicStream stream; float volume = 1.0f; bool active = false; };

struct Ps3Audio {
    bool ready = false;
    uint32_t port = 0;
    float* ring = nullptr;
    volatile uint64_t* readIndex = nullptr;
    uint32_t blocks = 0, blockFloats = 0, nextBlock = 0;
    MusicStream music;
    std::array<Voice, MaxVoices> voices;

    bool initialize() {
        if (cellAudioInit() != 0) return false;
        CellAudioPortParam param{};
        param.nChannel = CELL_AUDIO_PORT_2CH; param.nBlock = CELL_AUDIO_BLOCK_16;
        param.attr = CELL_AUDIO_PORTATTR_BGM | CELL_AUDIO_PORTATTR_INITLEVEL; param.level = 1.0f;
        if (cellAudioPortOpen(&param, &port) != 0) { cellAudioQuit(); return false; }
        CellAudioPortConfig config{};
        if (cellAudioGetPortConfig(port, &config) != 0) { cellAudioPortClose(port); cellAudioQuit(); return false; }
        ring = reinterpret_cast<float*>(static_cast<uintptr_t>(config.portAddr));
        readIndex = reinterpret_cast<volatile uint64_t*>(static_cast<uintptr_t>(config.readIndexAddr));
        blocks = static_cast<uint32_t>(config.nBlock); blockFloats = static_cast<uint32_t>(config.nChannel) * BlockSamples;
        std::memset(ring, 0, config.portSize);
        if (cellAudioPortStart(port) != 0) { cellAudioPortClose(port); cellAudioQuit(); return false; }
        nextBlock = static_cast<uint32_t>(*readIndex % blocks);
        ready = true; return true;
    }
    void shutdown() {
        music.close(); for (Voice& voice : voices) { voice.stream.close(); voice.active = false; }
        if (ready) { cellAudioPortStop(port); cellAudioPortClose(port); cellAudioQuit(); }
        ready = false; ring = nullptr;
    }
    void mixBlock(float* output, bool musicEnabled, bool soundEnabled) {
        std::memset(output, 0, blockFloats * sizeof(float));
        for (unsigned int s = 0; s < BlockSamples; ++s) {
            float left = 0.0f, right = 0.0f;
            if (musicEnabled) { float ml, mr; if (music.next(ml, mr)) { left += ml * 0.70f; right += mr * 0.70f; } }
            if (soundEnabled) for (Voice& voice : voices) if (voice.active) {
                float sl = 0.0f, sr = 0.0f;
                if (voice.stream.next(sl, sr)) { left += sl * voice.volume; right += sr * voice.volume; }
                else { voice.stream.close(); voice.active = false; }
            }
            output[s * 2] = std::max(-1.0f, std::min(1.0f, left)); output[s * 2 + 1] = std::max(-1.0f, std::min(1.0f, right));
        }
    }
    void update(bool musicEnabled, bool soundEnabled) {
        if (!ready) return;
        const uint32_t target = static_cast<uint32_t>((*readIndex + blocks / 2u) % blocks);
        while (nextBlock != target) {
            mixBlock(ring + nextBlock * blockFloats, musicEnabled, soundEnabled);
            nextBlock = (nextBlock + 1u) % blocks;
        }
    }
};

Ps3Audio gAudio;
}

AudioSystem& AudioSystem::instance() { static AudioSystem audio; return audio; }
bool AudioSystem::initialize() {
    if (initialized_) return true;
    loadSettings(); Localization::setLanguage(language_);
    initialized_ = gAudio.initialize();
    return initialized_;
}
void AudioSystem::shutdown() { if (!initialized_) return; gAudio.shutdown(); initialized_ = false; currentTrack_.clear(); currentMusic_.clear(); }
void AudioSystem::update() { if (initialized_) gAudio.update(musicEnabled_, soundEnabled_); }
std::string AudioSystem::pathFor(const char* file) const { return std::string(PD_ASSET_ROOT) + (file ? file : ""); }
std::string AudioSystem::musicFileFor(const char* file) const {
    if (!remixesEnabled_ || file == nullptr) return file ? file : "";
    if (std::strcmp(file, "theme.mp3") == 0) return "theme_remix.mp3";
    if (std::strcmp(file, "surface.mp3") == 0) return "surface_remix.mp3";
    if (std::strcmp(file, "game.mp3") == 0) return "game_remix.mp3";
    return file;
}
void AudioSystem::playMusic(const char* file, int) {
    if (!initialized_ || !file) return;
    const std::string selected = musicFileFor(file);
    if (currentMusic_ == selected) return;
    if (gAudio.music.open(pathFor(selected.c_str()).c_str())) { currentTrack_ = file; currentMusic_ = selected; }
}
void AudioSystem::playSound(const char* file, float volume) {
    if (!initialized_ || !soundEnabled_ || !file) return;
    for (Voice& voice : gAudio.voices) if (!voice.active) {
        voice.volume = std::max(0.0f, std::min(1.0f, volume));
        voice.active = voice.stream.open(pathFor(file).c_str(), false); return;
    }
}
void AudioSystem::setMusicEnabled(bool enabled) { musicEnabled_ = enabled; saveSettings(); }
void AudioSystem::setSoundEnabled(bool enabled) { soundEnabled_ = enabled; saveSettings(); if (!enabled) for (Voice& voice : gAudio.voices) { voice.stream.close(); voice.active = false; } }
void AudioSystem::setRemixesEnabled(bool enabled) { if (remixesEnabled_ == enabled) return; remixesEnabled_ = enabled; saveSettings(); if (!currentTrack_.empty()) { currentMusic_.clear(); playMusic(currentTrack_.c_str()); } }
void AudioSystem::setLanguage(Language language) { language_ = language; Localization::setLanguage(language_); saveSettings(); }
void AudioSystem::setCameraZoom(int zoom) { cameraZoom_ = std::max(0, std::min(3, zoom)); saveSettings(); }
void AudioSystem::loadSettings() {
    FILE* file = std::fopen(SettingsPath, "rb"); if (!file) return; SettingsDataV1 header{};
    if (std::fread(&header, 1, sizeof(header), file) == sizeof(header) && header.magic == SettingsMagic) {
        if (header.version == 1u) remixesEnabled_ = header.remixesEnabled != 0;
        else if (header.version == SettingsVersion) { std::rewind(file); SettingsData data{}; if (std::fread(&data, 1, sizeof(data), file) == sizeof(data)) { musicEnabled_ = data.musicEnabled != 0; soundEnabled_ = data.soundEnabled != 0; remixesEnabled_ = data.remixesEnabled != 0; language_ = data.language <= static_cast<unsigned char>(Language::Spanish) ? static_cast<Language>(data.language) : Language::English; cameraZoom_ = std::max(0, std::min(3, static_cast<int>(data.cameraZoom))); } }
    }
    std::fclose(file);
}
void AudioSystem::saveSettings() const {
    FILE* file = std::fopen(SettingsPath, "wb"); if (!file) return;
    const SettingsData data{SettingsMagic, SettingsVersion, static_cast<unsigned char>(musicEnabled_), static_cast<unsigned char>(soundEnabled_), static_cast<unsigned char>(remixesEnabled_), static_cast<unsigned char>(language_), static_cast<unsigned char>(cameraZoom_)};
    std::fwrite(&data, 1, sizeof(data), file); std::fclose(file);
}
} // namespace pdv
