#include "audio.h"

#include <math.h>

#define DRONE_BUFFER 1024

static Sound MakeToneSound(float frequency, float durationSec, float amplitude)
{
    Wave w = { 0 };
    w.frameCount = (unsigned int)(durationSec * 22050.0f);
    w.sampleRate = 22050;
    w.sampleSize = 16;
    w.channels = 1;

    short *samples = (short *)MemAlloc(w.frameCount * sizeof(short));
    for (unsigned int i = 0; i < w.frameCount; i++)
    {
        float t = (float)i / (float)w.sampleRate;
        float env = 1.0f - (t / durationSec);
        float s = sinf(2.0f * 3.1415926f * frequency * t) * env * amplitude;
        samples[i] = (short)(s * 32767.0f);
    }

    w.data = samples;
    Sound sound = LoadSoundFromWave(w);
    UnloadWave(w);
    return sound;
}

void AudioSystemInit(AudioSystem *audio)
{
    audio->bird = MakeToneSound(1440.0f, 0.12f, 0.26f);
    audio->step = MakeToneSound(94.0f, 0.05f, 0.55f);
    audio->clock = MakeToneSound(1600.0f, 0.03f, 0.28f);
    audio->soundsReady = true;

    audio->droneStream = LoadAudioStream(44100, 32, 1);
    PlayAudioStream(audio->droneStream);
    audio->droneReady = true;
    audio->dronePhase = 0.0f;

    audio->birdTimer = 0.4f;
    audio->clockTimer = 0.9f;
    audio->echoPending = false;
    audio->echoTimer = 0.0f;
}

void AudioSystemUpdate(AudioSystem *audio, float dt, bool inside, float conditionNorm)
{
    audio->birdTimer -= dt;
    if (audio->birdTimer <= 0.0f)
    {
        PlaySound(audio->bird);
        audio->birdTimer = 4.3f;
    }

    if (inside)
    {
        audio->clockTimer -= dt;
        if (audio->clockTimer <= 0.0f)
        {
            PlaySound(audio->clock);
            audio->clockTimer = 0.95f + ((float)GetRandomValue(0, 100) / 100.0f) * 0.10f;
        }
    }

    if (audio->echoPending)
    {
        audio->echoTimer -= dt;
        if (audio->echoTimer <= 0.0f)
        {
            SetSoundVolume(audio->step, 0.35f);
            PlaySound(audio->step);
            SetSoundVolume(audio->step, 1.0f);
            audio->echoPending = false;
        }
    }

    if (audio->droneReady && IsAudioStreamProcessed(audio->droneStream))
    {
        static float buffer[DRONE_BUFFER] = { 0 };
        float loudness = 0.015f + (1.0f - conditionNorm) * 0.06f;

        for (int i = 0; i < DRONE_BUFFER; i++)
        {
            float a = sinf(audio->dronePhase);
            float b = sinf(audio->dronePhase * 0.47f + 1.1f);
            buffer[i] = (a * 0.7f + b * 0.3f) * loudness;

            audio->dronePhase += 2.0f * 3.1415926f * 48.0f / 44100.0f;
            if (audio->dronePhase > 2.0f * 3.1415926f) audio->dronePhase -= 2.0f * 3.1415926f;
        }

        UpdateAudioStream(audio->droneStream, buffer, DRONE_BUFFER);
    }
}

void AudioSystemPlayFootstep(AudioSystem *audio)
{
    PlaySound(audio->step);
    audio->echoPending = true;
    audio->echoTimer = 0.40f;
}

void AudioSystemUnload(AudioSystem *audio)
{
    if (audio->droneReady)
    {
        StopAudioStream(audio->droneStream);
        UnloadAudioStream(audio->droneStream);
        audio->droneReady = false;
    }

    if (audio->soundsReady)
    {
        UnloadSound(audio->bird);
        UnloadSound(audio->step);
        UnloadSound(audio->clock);
        audio->soundsReady = false;
    }
}
