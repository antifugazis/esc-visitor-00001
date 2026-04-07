#ifndef AUDIO_H
#define AUDIO_H

#include "raylib.h"

#include <stdbool.h>

typedef struct AudioSystem {
    Sound bird;
    Sound step;
    Sound clock;
    bool soundsReady;

    AudioStream droneStream;
    bool droneReady;
    float dronePhase;

    float birdTimer;
    float clockTimer;

    bool echoPending;
    float echoTimer;
} AudioSystem;

void AudioSystemInit(AudioSystem *audio);
void AudioSystemUpdate(AudioSystem *audio, float dt, bool inside, float conditionNorm);
void AudioSystemPlayFootstep(AudioSystem *audio);
void AudioSystemUnload(AudioSystem *audio);

#endif
