#pragma once

#include <SDL3/SDL_audio.h>
#include <string>

namespace core {

class AudioManager {

public:
    enum class Sound { PaddleHit, WallHit, Score, Count };

    AudioManager();
    ~AudioManager();

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    static AudioManager& get();

    void play(Sound sound);

    void play_music(const std::string& path);
    void stop_music();
    void set_music_volume(float volume);

private:
    struct Clip {
        SDL_AudioSpec spec      = {};
        Uint8*        buf       = nullptr;
        Uint32        len       = 0;
        SDL_AudioStream* stream = nullptr;
    };

    void load(Sound sound, const std::string& path);

    static AudioManager* s_instance;
    static void music_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount);
    void refill_music(SDL_AudioStream* stream, int additional_amount);

    SDL_AudioDeviceID m_device            = 0;
    Clip              m_clips[static_cast<int>(Sound::Count)];

    SDL_AudioStream*  m_music_stream      = nullptr;
    short*            m_music_buf         = nullptr;
    int               m_music_samples     = 0;
    int               m_music_channels    = 0;
    int               m_music_sample_rate = 0;
    float             m_music_volume      = 1.0f;

};

}
