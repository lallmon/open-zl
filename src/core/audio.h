#ifndef AUDIO_H
#define AUDIO_H

#include <fstream>

#include "portaudio.h"

class AudioController;

#pragma pack(push,4)
struct WavHeaderData {
    char ChunkId[4];
    int32_t ChunkSize;
    char Format[4];
    char Subchunk1ID[4];
    int32_t Subchunk1Size;
    int16_t AudioFormat;
    int16_t NumChannels;
    int32_t SampleRate;
    int32_t ByteRate;
    int16_t BlockAlign;
    int16_t BitsPerSample;
    char Subchunk2ID[4];
    int32_t Subchunk2Size;

    bool isUncompressed() const { return AudioFormat == 1; }
    int sampleCount() const { return Subchunk2Size / (NumChannels * BitsPerSample / 8); }
};
#pragma pack(pop)

class AudioReference
{
    bool failed = false;

    std::ifstream file;

    WavHeaderData header;

    void readHeader();
protected:
    int getSampleCount() const;
    void setPosition(int sample);
    void readToBuffer(int16_t *buffer, int samplecount, int channelcount, uint8_t scaling_factor, bool loop);

    friend AudioController;
public:

    bool ready() const;
    void load(const char * resource_path, const char * fname);
};

class AudioController
{
    AudioReference * target;

    bool m_loops = false;

    int position = 0;

    float fade_vol = 1;
    float fade_velocity = 1.0f;
    enum Mode {
        FadingOut,
        FadingIn
    } mode = FadingIn;

    uint8_t calculateVolume(float scaling) const;
public:
    void play(AudioReference * ref, bool looping = false);

    void fadeOut(float duration);
    void fadeIn(float duration);
    float fadeVolume() const;
    void updateFade(float t);

    bool playing() const;
    bool playing(AudioReference * ref) const { return target == ref; }

    bool loops() const { return m_loops; }
    void stop();

    void readToBuffer(int16_t *buffer, int samplecount, int channels, float scaling);

};

class Audio {

    static PaStream * s_stream;

    static constexpr int channel_count = 5;
    static AudioController s_clips[channel_count];

    static constexpr float mus_weight = 0.4f;
    static constexpr float sfx_weight = (1.0f - mus_weight)/float(channel_count - 1);

    static float master_volume;
    static float sfx_volume;
    static float music_volume;

    static AudioReference *getReference(std::string name);
    static bool checkFailure(PaError error);
    static int updateSound( const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData );
public:
    static bool init();
    static void terminate();    

    static void playSFX(std::string name);
    static void playMusic(std::string name, float transition_time);
};

#endif // AUDIO_H
