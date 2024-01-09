#include "audio.h"

#include <iostream>
#include <unordered_map>

#include "core/resourcelocator.h"

float Audio::master_volume = 1.0f;
float Audio::sfx_volume = 1.0f;
float Audio::music_volume = 1.0f;

constexpr int audioChannels = 2;
constexpr float audioFreq = 44100;

void AudioReference::readHeader()
{
    file.seekg(0);
    file.read(reinterpret_cast<char*>(&header), sizeof(WavHeaderData));
}

int AudioReference::getSampleCount() const
{
    return header.sampleCount();
}

void AudioReference::setPosition(int sample)
{
    file.seekg((sample % getSampleCount()) * sizeof(uint16_t) + 44);
}

void AudioReference::readToBuffer(int16_t *buffer, int samplecount, int channelcount, uint8_t scaling_factor, bool loop)
{
    if (!ready()) {
        return;
    }
    int16_t sample;

    if (channelcount == header.NumChannels) {
        samplecount *= channelcount;
        while (samplecount > 0) {
            if (file.peek() == EOF) {
                if (!loop) return;
                else file.seekg(44);
            }
            file.read(reinterpret_cast<char*>(&sample), sizeof(int16_t));
            *buffer++ += int16_t(int(sample) * int(scaling_factor) >> 8);
            samplecount--;
        }
    } else if (header.NumChannels < channelcount) {
        int mult = channelcount / header.NumChannels;
        while (samplecount > 0) {
            if (file.peek() == EOF) {
                if (!loop) return;
                else file.seekg(44);
            }
            file.read(reinterpret_cast<char*>(&sample), sizeof(int16_t));
            int16_t s = int16_t(int(sample) * int(scaling_factor) >> 8);
            for(int i = 0; i < mult; ++i) {
                *buffer++ += s;
            }
            samplecount--;
        }
    } else {
        std::cerr << "ERROR: Mixing channels from file down not supported." << std::endl;
        std::cerr << "       file channel count: " << header.NumChannels << std::endl;
        std::cerr << "       buffer channel count: " << channelcount << std::endl;
    }
}

bool AudioReference::ready() const
{
    return !failed && file.is_open() && header.sampleCount() > 0 && header.isUncompressed();
}

void AudioReference::load(std::string filename)
{
    if (failed) return;

    file.open(filename, std::ifstream::binary | std::ifstream::in);

    if (!file.good()) {
        std::cerr << "ERROR: Couldn't open " << filename.c_str() << std::endl;
        failed = true;
        return;
    }

    readHeader();
}

void AudioController::play(AudioReference *ref, bool looping)
{
    m_loops = looping;
    mode = FadingIn;
    fade_vol = 0;
    fade_velocity = 10;
    if (m_loops && ref == target) return;
    target = ref;
    position = 0;
}

void AudioController::fadeOut(float duration)
{
    if (duration <= 0) {
        fade_vol = 0;
    } else {
        fade_velocity = 1/duration;
    }
    mode = FadingOut;
}

void AudioController::fadeIn(float duration)
{
    if (duration <= 0) {
        fade_vol = 1;
    } else {
        fade_velocity = 1/duration;
    }
    mode = FadingIn;
}

float AudioController::fadeVolume() const
{
    return fade_vol;
}

void AudioController::updateFade(float t)
{
    if (mode == FadingIn) {
        fade_vol += fade_velocity * t;
        if (fade_vol > 1) fade_vol = 1;
    } else {
        fade_vol -= fade_velocity * t;
        if (fade_vol < 0) {
            stop();
            fade_vol = 0;
        }
    }
}

bool AudioController::playing() const
{
    if (target == nullptr) return false;
    if (m_loops) return true;
    return position < target->getSampleCount();
}

void AudioController::stop()
{
    target = nullptr;
}

void AudioController::readToBuffer(int16_t *buffer, int samplecount, int channels, float scaling)
{
    if (target == nullptr || scaling <= 0.0f) return;
    target->setPosition(position);
    target->readToBuffer(buffer, samplecount, channels, int(255.0f * scaling), m_loops);
    position += samplecount * target->header.NumChannels;
    if (!playing()) target = nullptr;
}


AudioController Audio::s_clips[];

PaStream * Audio::s_stream;

bool Audio::init()
{
    if (checkFailure(Pa_Initialize())) return false;

    if (Pa_GetDeviceCount() < 1 ) {
        std::cout << "No audio devices found!\n";
        return false;
    }

    PaError errorCode;
    errorCode = Pa_OpenDefaultStream(&s_stream,
                                     0,             // input channel count
                                     2,             // output channel count
                                     paInt16,       // output type
                                     44100,         // sample rate
                                     512,          // frames per buffer
                                     &updateSound,  // callback
                                     nullptr);

    if (checkFailure(errorCode)) {
        std::cout << "ERROR: Could not open stream!" << std::endl;
        return false;
    }
    if (checkFailure(Pa_StartStream(s_stream))) {
        std::cout << "ERROR: Could not start stream!" << std::endl;
        return false;
    }
    return true;
}

void Audio::terminate()
{
    if (checkFailure(Pa_StopStream(s_stream))) return;
    if (checkFailure(Pa_CloseStream(s_stream))) return;
    Pa_Terminate();
}

AudioReference *Audio::getReference(std::string name)
{
    static std::unordered_map<std::string, AudioReference> refs;
    if (refs.find(name) == refs.end()) {
        refs[name] = AudioReference();
        refs.at(name).load(ResourceLocator::getPathWAV(name));
    }
    if (!refs.at(name).ready()) return nullptr;
    return &refs.at(name);
}

bool Audio::checkFailure(PaError error)
{
    if (error != paNoError) {
        Pa_Terminate();
        std::cerr << "Error while using PortAudio (" << error << ")" << std::endl;
        std::cerr << "  " << Pa_GetErrorText(error) << std::endl;
        return true;
    }
    return false;
}

int Audio::updateSound ( const void*,                     // inputBuffer
                         void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo*, // timing data (doesn't work with alsa)
                         PaStreamCallbackFlags,           // status
                         void *)                          // userData
{
    int16_t *buffer = reinterpret_cast<int16_t*>(outputBuffer);
    std::memset(buffer, 0, framesPerBuffer * 2 * audioChannels);
    for(int i = 0; i < channel_count; ++i) {
        if (s_clips[i].playing()) {
            float weight = s_clips[i].loops() ? music_volume * mus_weight : sfx_volume * sfx_weight;
            weight *= s_clips[i].fadeVolume();
            s_clips[i].readToBuffer(buffer, framesPerBuffer, audioChannels, master_volume * weight);
            s_clips[i].updateFade(float(framesPerBuffer) / 44100.0f);
        }
    }
    return 0;
}

void Audio::playSFX(std::string name)
{
    if (!Pa_IsStreamActive(s_stream)) {
        std::cerr << "ERROR: No active audio stream" << std::endl;
        return;
    }

    AudioReference *ref = getReference(name);
    if (ref == nullptr) return;
    for (int i = 0; i < channel_count; i++) {
        if (!s_clips[i].playing()) {
            s_clips[i].play(ref, false);
            return;
        }
    }
}

void Audio::playMusic(std::string name, float transition_time)
{
    if (!Pa_IsStreamActive(s_stream)) {
        std::cerr << "ERROR: No active audio stream" << std::endl;
        return;
    }

    for (int i = 0; i < channel_count; i++) {
        if (s_clips[i].loops()) s_clips[i].fadeOut(transition_time);
    }

    AudioReference *ref = getReference(name);
    if (ref == nullptr) return;

    for (int i = 0; i < channel_count; i++) {
        if (!s_clips[i].playing()) {
            s_clips[i].play(ref, true);
            s_clips[i].fadeIn(transition_time);
            return;
        }
    }
}
