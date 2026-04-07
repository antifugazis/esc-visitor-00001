#ifndef TONE_H
#define TONE_H

#include "raylib.h"

typedef struct ToneSystem {
    AudioStream stream;
    bool ready;

    int sampleRate;
    float phase;
    float time;

    float pulseSpacing;
    float pulseDuration;
    float targetSpacing;

    float movementNorm;
} ToneSystem;

void ToneInit(ToneSystem *tone);
void ToneUpdate(ToneSystem *tone, float dt, float playerSpeed);
void ToneUnload(ToneSystem *tone);

#endif
