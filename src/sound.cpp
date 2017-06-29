#include "xaudio2.h"
#pragma comment(lib, "xaudio2.lib")

#include "../lib/stb_vorbis.h"

IXAudio2* x_audio;
IXAudio2MasteringVoice* mastering_voice;

struct Sound {
    utf8* file_name   = null;
    u16 channels      = 0;
    u32 sample_rate   = 0;
    u8* samples       = null;
    u32 samples_count = 0;
};

Sound load_sound(utf8* file_name) {
    Sound sound;
    sound.file_name = file_name;

    sound.samples_count = stb_vorbis_decode_filename(
        (char*)   sound.file_name, 
        (int*)    &sound.channels, 
        (int*)    &sound.sample_rate, 
        (short**) &sound.samples);

    assert(sound.samples_count);
    
    printf("Loaded sound '%s'\n", sound.file_name);
    return sound;
}

struct Playing_Sound : public IXAudio2VoiceCallback {
    IXAudio2SourceVoice* source_voice = null;
    Sound* sound = null;
    bool is_playing = true;

    void OnStreamEnd() {
        is_playing = false;
    }

    void OnVoiceProcessingPassEnd() {}
    void OnVoiceProcessingPassStart(UINT32 samples_required) {}
    void OnBufferEnd(void * buffer_context) {}
    void OnBufferStart(void * buffer_context) {}
    void OnLoopEnd(void * buffer_context) {}
    void OnVoiceError(void * buffer_context, HRESULT error) {}
};

Bucket_Array<Playing_Sound, 32> playing_sounds;
HANDLE playing_sounds_mutex;

Playing_Sound* play_sound(Sound* sound, float volume = 1.0f, bool loop = false) {
    WaitForSingleObject(playing_sounds_mutex, INFINITE);

    WAVEFORMATEX source_format = {
        WAVE_FORMAT_PCM,
        sound->channels,
        sound->sample_rate,
        sound->sample_rate * sound->channels * 2,
        (u16) (sound->channels * 2),
        16,
        0
    };

    Playing_Sound* playing_sound = next(&playing_sounds);
    playing_sound->sound = sound;

    x_audio->CreateSourceVoice(
        &playing_sound->source_voice, 
        &source_format, 
        0, 
        XAUDIO2_DEFAULT_FREQ_RATIO, 
        playing_sound, 
        null, 
        null);

    XAUDIO2_BUFFER buffer = {
        XAUDIO2_END_OF_STREAM,
        sound->samples_count * sound->channels * 2,
        sound->samples,
        0,
        0,
        0,
        0,
        loop ? XAUDIO2_LOOP_INFINITE : (u32) 0,
        null
    };

    playing_sound->source_voice->SubmitSourceBuffer(&buffer);
    playing_sound->source_voice->SetVolume(volume);
    playing_sound->source_voice->Start(0);

    ReleaseMutex(playing_sounds_mutex);

    return playing_sound;
}

void set_volume(Playing_Sound* playing_sound, float volume) {
    playing_sound->source_voice->SetVolume(volume);
}

Sound music_sound;
Sound laser_01_sound;
Sound laser_02_sound;
Sound spawn_sound;
Sound kill_01_sound;
Sound kill_02_sound;

void init_sound() {
    XAudio2Create(&x_audio, 0, XAUDIO2_DEFAULT_PROCESSOR);
    x_audio->CreateMasteringVoice(&mastering_voice);

    mastering_voice->SetVolume(0.25f);

    music_sound    = load_sound("data/sounds/music.ogg");
    laser_01_sound = load_sound("data/sounds/laser_01.ogg");
    laser_02_sound = load_sound("data/sounds/laser_02.ogg");
    spawn_sound    = load_sound("data/sounds/spawn.ogg");
    kill_01_sound  = load_sound("data/sounds/kill_01.ogg");
    kill_02_sound  = load_sound("data/sounds/kill_02.ogg");

    playing_sounds_mutex = CreateMutex(null, false, null);
}

void update_sound() {
    WaitForSingleObject(playing_sounds_mutex, INFINITE);

    for_each (Playing_Sound* playing_sound, &playing_sounds) {
        if (playing_sound->is_playing) continue;
        
        playing_sound->source_voice->DestroyVoice();
        remove(&playing_sounds, iterator.current);
    }

    ReleaseMutex(playing_sounds_mutex);
}