#include "mirror.h"

#include <string.h>

static Camera3D SampleDelayedCamera(const MirrorSystem *mirror, float delaySeconds, const Camera3D *fallback)
{
    if (mirror->historyCount <= 0) return *fallback;

    float target = mirror->elapsed - delaySeconds;
    for (int i = 0; i < mirror->historyCount; i++)
    {
        int idx = (mirror->historyHead - i + 127) % 128;
        if (mirror->historyTime[idx] <= target) return mirror->history[idx];
    }

    return mirror->history[(mirror->historyHead - mirror->historyCount + 128) % 128];
}

void MirrorInit(MirrorSystem *mirror, int width, int height)
{
    memset(mirror, 0, sizeof(*mirror));

    mirror->reflectionTarget = LoadRenderTexture(width, height);
    SetTextureFilter(mirror->reflectionTarget.texture, TEXTURE_FILTER_POINT);

    Mesh m = GenMeshPlane(1.2f, 1.6f, 1, 1);
    mirror->surface = LoadModelFromMesh(m);
    mirror->surface.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = mirror->reflectionTarget.texture;

    mirror->ready = true;
}

void MirrorUpdate(MirrorSystem *mirror, float dt, const Camera3D *playerCamera, float totalPlaytimeSeconds)
{
    if (!mirror->ready) return;

    mirror->elapsed += dt;

    mirror->history[mirror->historyHead] = *playerCamera;
    mirror->historyTime[mirror->historyHead] = mirror->elapsed;
    mirror->historyHead = (mirror->historyHead + 1) % 128;
    if (mirror->historyCount < 128) mirror->historyCount++;

    mirror->postureOffset = (totalPlaytimeSeconds >= 300.0f) ? 0.06f : 0.0f;
    mirror->squashX = (totalPlaytimeSeconds >= 600.0f) ? 0.92f : 1.0f;
}

Camera3D MirrorGetReflectionCamera(const MirrorSystem *mirror, const Camera3D *playerCamera)
{
    float delay = (mirror->elapsed >= 120.0f) ? 0.1f : 0.0f;
    Camera3D c = SampleDelayedCamera(mirror, delay, playerCamera);

    c.position.x *= -1.0f;
    c.target.x *= -1.0f;
    c.position.y += mirror->postureOffset;
    c.target.y += mirror->postureOffset;

    return c;
}

void MirrorBeginReflection(const MirrorSystem *mirror)
{
    if (!mirror->ready) return;
    BeginTextureMode(mirror->reflectionTarget);
    ClearBackground((Color){ 12, 12, 14, 255 });
}

void MirrorEndReflection(void)
{
    EndTextureMode();
}

void MirrorDraw(const MirrorSystem *mirror)
{
    if (!mirror->ready) return;

    Vector3 pos = { -2.9f, 1.4f, -3.85f };
    DrawModelEx(mirror->surface, pos, (Vector3){ 0.0f, 1.0f, 0.0f }, 90.0f, (Vector3){ mirror->squashX, 1.0f, 1.0f }, WHITE);
    DrawCubeWires((Vector3){ -2.9f, 1.4f, -3.9f }, 1.22f, 1.62f, 0.04f, (Color){ 76, 70, 66, 255 });
}

void MirrorUnload(MirrorSystem *mirror)
{
    if (!mirror->ready) return;
    UnloadModel(mirror->surface);
    UnloadRenderTexture(mirror->reflectionTarget);
    mirror->ready = false;
}
