#ifndef WORKER_H
#define WORKER_H

#include "raylib.h"

typedef enum WorkerAnim {
    WORKER_ANIM_START_WALK = 0,
    WORKER_ANIM_WALK,
    WORKER_ANIM_TURN,
    WORKER_ANIM_TURN_RIGHT,
    WORKER_ANIM_TURN_LEFT,
    WORKER_ANIM_RUN,
    WORKER_ANIM_COUNT
} WorkerAnim;

typedef struct WorkerClip {
    ModelAnimation *anims;
    int count;
} WorkerClip;

typedef struct WorkerRig {
    Model model;
    bool ready;
    float baseScale;
    WorkerClip clips[WORKER_ANIM_COUNT];
} WorkerRig;

void WorkerInit(WorkerRig *worker, const char *modelPath, const char *animDir);
void WorkerSetShader(WorkerRig *worker, Shader shader);
void WorkerUnload(WorkerRig *worker);
int WorkerClipFrameCount(const WorkerRig *worker, WorkerAnim anim);
void WorkerDrawAnimated(WorkerRig *worker, WorkerAnim anim, float frame, Vector3 position, float yawDegrees, float scale, Color tint);

#endif
