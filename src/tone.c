#include "tone.h"

#include <math.h>

#define TONE_BUFFER 1024

static float Clamp01(float v)
{
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

void ToneInit(ToneSystem *tone)
{
    tone->sampleRate = 44100;
    tone->stream = LoadAudioStream((unsigned int)tone->sampleRate, 32, 1);
    PlayAudioStream(tone->stream);

    tone->ready = true;
    tone->phase = 0.0f;
    tone->time = 0.0f;
    tone->pulseSpacing = 1.4f;
    tone->pulseDuration = 0.23f;
    tone->targetSpacing = 1.4f;
    tone->movementNorm = 0.0f;
}

void ToneUpdate(ToneSystem *tone, float dt, float playerSpeed)
{
    if (!tone->ready) return;

    tone->movementNorm = Clamp01(playerSpeed / 3.6f);
    tone->targetSpacing = 1.8f - tone->movementNorm * 1.45f;
    tone->pulseSpacing += (tone->targetSpacing - tone->pulseSpacing) * (dt * 3.0f);

    if (IsAudioStreamProcessed(tone->stream))
    {
        static float buffer[TONE_BUFFER] = { 0 };

        const float freq = 76.0f + 10.0f * tone->movementNorm;
        const float amp = 0.075f;

        for (int i = 0; i < TONE_BUFFER; i++)
        {
            float pulseTime = fmodf(tone->time, tone->pulseSpacing);
            float env = 0.0f;
            if (pulseTime < tone->pulseDuration)
            {
                float x = pulseTime / tone->pulseDuration;
                env = sinf(x * 3.1415926f);
            }

            float sample = sinf(tone->phase) * env * amp;
            buffer[i] = sample;

            tone->phase += 2.0f * 3.1415926f * freq / (float)tone->sampleRate;
            if (tone->phase > 2.0f * 3.1415926f) tone->phase -= 2.0f * 3.1415926f;
            tone->time += 1.0f / (float)tone->sampleRate;
        }

        UpdateAudioStream(tone->stream, buffer, TONE_BUFFER);
    }
}

void ToneUnload(ToneSystem *tone)
{
    if (!tone->ready) return;
    StopAudioStream(tone->stream);
    UnloadAudioStream(tone->stream);
    tone->ready = false;
}
