#include <xaudio2.h>

#pragma comment(lib, "xaudio2.lib")

IXAudio2* xaudio;

IXAudio2MasteringVoice* mastering_voice;
IXAudio2SourceVoice*    source_voice;

void init_sound() {
    // HRESULT result = XAudio2Create(&xaudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
    // if (FAILED(result)) {
    //     print("Failed to initialize XAudio\n");
    //     return;
    // }

    // result = xaudio->CreateMasteringVoice(&mastering_voice);
    // if (FAILED(result)) {
    //     print("Failed to create XAudio mastering voice\n");
    //     return;
    // }

    // result = xaudio->CreateSourceVoice(&source_voice, &format);
    // if (FAILED(result)) {
    //     print("Failed to create XAudio source voice\n");
    //     return;
    // }

    // result = source_voice->SubmitSourceBuffer(&buffer);
    // if (FAILED(result)) {
    //     print("Failed to submit buffer to XAudio source voice\n");
    //     return;
    // }

    // result = source_voice->Start(0);
    // if (FAILED(result)) {
    //     print("Failed to start XAudio source voice\n");
    // }
}