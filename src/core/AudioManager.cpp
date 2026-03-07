#include "AudioManager.hpp"
#include "Logger.hpp"

extern "C" {
    int stb_vorbis_decode_filename(const char* filename, int* channels, int* sample_rate, short** output);
}

#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>

namespace core {

AudioManager* AudioManager::s_instance = nullptr;

AudioManager& AudioManager::get() {
    return *s_instance;
}

AudioManager::AudioManager() {
    s_instance = this;
    m_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!m_device) {
        Log::error("SDL_OpenAudioDevice failed: {}", SDL_GetError());
        return;
    }

    const char* base = SDL_GetBasePath();
    const std::string sounds = std::string(base) + "assets/sounds/";

    load(Sound::PaddleHit, sounds + "paddle_hit.wav");
    load(Sound::WallHit,   sounds + "wall_hit.wav");
    load(Sound::Score,     sounds + "score.wav");
}

AudioManager::~AudioManager() {
    s_instance = nullptr;
    stop_music();

    for (auto& clip : m_clips) {
        if (clip.stream) SDL_DestroyAudioStream(clip.stream);
        if (clip.buf)    SDL_free(clip.buf);
    }

    if (m_device) SDL_CloseAudioDevice(m_device);
}

void AudioManager::load(Sound sound, const std::string& path) {
    auto& clip = m_clips[static_cast<int>(sound)];

    if (!SDL_LoadWAV(path.c_str(), &clip.spec, &clip.buf, &clip.len)) {
        Log::error("SDL_LoadWAV failed ({}): {}", path, SDL_GetError());
        return;
    }

    clip.stream = SDL_CreateAudioStream(&clip.spec, nullptr);
    if (!clip.stream) {
        Log::error("SDL_CreateAudioStream failed: {}", SDL_GetError());
        return;
    }

    SDL_BindAudioStream(m_device, clip.stream);
}

void AudioManager::play(Sound sound) {
    if (!m_device) return;

    auto& clip = m_clips[static_cast<int>(sound)];
    if (!clip.stream || !clip.buf) return;

    SDL_PutAudioStreamData(clip.stream, clip.buf, clip.len);
}

void AudioManager::play_music(const std::string& path) {
    stop_music();

    int channels;
    int sample_rate;

    m_music_samples = stb_vorbis_decode_filename(
        path.c_str(),
        &channels,
        &sample_rate,
        &m_music_buf
    );

    if (m_music_samples < 0) {
        Log::error("stb_vorbis failed to load: {}", path);
        return;
    }

    m_music_channels    = channels;
    m_music_sample_rate = sample_rate;

    SDL_AudioSpec spec;
    spec.format    = SDL_AUDIO_S16;
    spec.channels  = channels;
    spec.freq      = sample_rate;

    m_music_stream = SDL_CreateAudioStream(&spec, nullptr);
    if (!m_music_stream) {
        Log::error("SDL_CreateAudioStream failed for music: {}", SDL_GetError());
        return;
    }

    SDL_SetAudioStreamGetCallback(m_music_stream, music_callback, this);
    SDL_BindAudioStream(m_device, m_music_stream);
    set_music_volume(m_music_volume);
}

void AudioManager::stop_music() {
    if (m_music_stream) {
        SDL_DestroyAudioStream(m_music_stream);
        m_music_stream = nullptr;
    }

    if (m_music_buf) {
        free(m_music_buf);
        m_music_buf = nullptr;
    }

    m_music_samples = 0;
}

void AudioManager::set_music_volume(float volume) {
    m_music_volume = volume;

    if (m_music_stream)
        SDL_SetAudioStreamGain(m_music_stream, volume);
}

void AudioManager::music_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) {
    (void)total_amount;
    static_cast<AudioManager*>(userdata)->refill_music(stream, additional_amount);
}

void AudioManager::refill_music(SDL_AudioStream* stream, int additional_amount) {
    if (!m_music_buf || m_music_samples <= 0) return;

    const int bytes_per_sample = sizeof(short) * m_music_channels;
    const int total_bytes      = m_music_samples * bytes_per_sample;

    int remaining = additional_amount;
    while (remaining > 0) {
        const int chunk = remaining < total_bytes
                            ? remaining
                            : total_bytes;

        SDL_PutAudioStreamData(stream, m_music_buf, chunk);
        remaining -= chunk;
    }
}


}
