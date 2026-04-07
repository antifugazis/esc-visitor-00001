#ifndef MIRROR_H
#define MIRROR_H

#include "raylib.h"

typedef struct MirrorSystem {
    RenderTexture2D reflectionTarget;
    Model surface;
    bool ready;

    Camera3D history[128];
    float historyTime[128];
    int historyCount;
    int historyHead;

    float elapsed;
    float squashX;
    float postureOffset;
} MirrorSystem;

void MirrorInit(MirrorSystem *mirror, int width, int height);
void MirrorUpdate(MirrorSystem *mirror, float dt, const Camera3D *playerCamera, float totalPlaytimeSeconds);
Camera3D MirrorGetReflectionCamera(const MirrorSystem *mirror, const Camera3D *playerCamera);
void MirrorBeginReflection(const MirrorSystem *mirror);
void MirrorEndReflection(void);
void MirrorDraw(const MirrorSystem *mirror);
void MirrorUnload(MirrorSystem *mirror);

#endif
