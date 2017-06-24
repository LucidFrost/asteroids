#include "xaudio2.h"
#pragma comment(lib, "xaudio2.lib")

#include "../lib/stb_vorbis.h"

IXAudio2* x_audio;
IXAudio2MasteringVoice* mastering_voice;

struct Sound {
    int channels    = 0;
    int sample_rate = 0;

    u8* samples       = NULL;
    int samples_count = 0;
};

Sound load_sound(char* file_name) {
    Sound sound;

    sound.samples_count = stb_vorbis_decode_filename(file_name, &sound.channels, &sound.sample_rate, (i16**) &sound.samples);
    assert(sound.samples_count);
    
    printf("Loaded sound '%s'\n", file_name);

    return sound;
}

// @note: Apparently it is considered invalid to destroy a voice
// in the callback, but it still seems to work just fine?
void stop_sound(struct Playing_Sound* playing_sound);

struct Playing_Sound : public IXAudio2VoiceCallback {
    IXAudio2SourceVoice* source_voice = NULL;

    void OnStreamEnd() {
        stop_sound(this);
    }

    void OnVoiceProcessingPassEnd() {}
    void OnVoiceProcessingPassStart(UINT32 samples_required) {}
    void OnBufferEnd(void * buffer_context) {}
    void OnBufferStart(void * buffer_context) {}
    void OnLoopEnd(void * buffer_context) {}
    void OnVoiceError(void * buffer_context, HRESULT error) {}
};

Playing_Sound playing_sound_buffer[32];
bool playing_sound_buffer_mask[count_of(playing_sound_buffer)];

Playing_Sound* play_sound(Sound* sound, float volume = 1.0f, bool loop = false) {
    WAVEFORMATEX source_format = {
        WAVE_FORMAT_PCM,
        (u16) sound->channels,
        (u32) sound->sample_rate,
        (u32) (sound->sample_rate * sound->channels * 2),
        (u16) (sound->channels * 2),
        16,
        0
    };

    Playing_Sound* playing_sound = NULL;
    for (int i = 0; i < count_of(playing_sound_buffer); i++) {
        if (playing_sound_buffer_mask[i]) continue;

        playing_sound = &playing_sound_buffer[i];
        playing_sound_buffer_mask[i] = true;

        break;
    }

    assert(playing_sound);

    IXAudio2SourceVoice* source_voice;
    x_audio->CreateSourceVoice(&source_voice, &source_format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, playing_sound, NULL, NULL);

    playing_sound->source_voice = source_voice;

    XAUDIO2_BUFFER buffer = {
        XAUDIO2_END_OF_STREAM,
        (u32) (sound->samples_count * sound->channels * 2),
        sound->samples,
        0,
        0,
        0,
        0,
        loop ? XAUDIO2_LOOP_INFINITE : (u32) 0,
        NULL
    };

    source_voice->SubmitSourceBuffer(&buffer);
    source_voice->SetVolume(volume);
    source_voice->Start(0);

    return playing_sound;
}

void stop_sound(Playing_Sound* playing_sound) {
    int index = (int) (playing_sound - playing_sound_buffer);
    
    assert(index < count_of(playing_sound_buffer));
    assert(playing_sound_buffer_mask[index]);
    
    playing_sound->source_voice->DestroyVoice();
    playing_sound->source_voice = NULL;

    playing_sound_buffer_mask[index] = false;
}

void set_volume(Playing_Sound* playing_sound, float volume) {
    playing_sound->source_voice->SetVolume(volume);
}

Sound music_sound;
Sound laser_01_sound;
Sound laser_02_sound;
Sound spawn_sound;
Sound kill_sound;

void init_sound() {
    XAudio2Create(&x_audio, 0, XAUDIO2_DEFAULT_PROCESSOR);
    x_audio->CreateMasteringVoice(&mastering_voice);

    music_sound    = load_sound("data/sounds/music.ogg");
    laser_01_sound = load_sound("data/sounds/laser_01.ogg");
    laser_02_sound = load_sound("data/sounds/laser_02.ogg");
    spawn_sound    = load_sound("data/sounds/spawn.ogg");
    kill_sound     = load_sound("data/sounds/kill.ogg");
}