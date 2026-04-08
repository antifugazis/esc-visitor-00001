#include "worker.h"

#include <string.h>

static void LoadClip(WorkerRig *worker, WorkerAnim id, const char *path)
{
    worker->clips[id].anims = LoadModelAnimations(path, &worker->clips[id].count);
}

void WorkerInit(WorkerRig *worker, const char *modelPath, const char *animDir)
{
    memset(worker, 0, sizeof(*worker));

    if (FileExists(modelPath)) worker->model = LoadModel(modelPath);

    // Some FBX exports store only skeleton/anim in the "character" file.
    // Fallback to animation FBX files that often include mesh data.
    if (worker->model.meshCount <= 0)
    {
        if (worker->model.meshCount == 0 && worker->model.materialCount > 0) UnloadModel(worker->model);

        const char *fallbacks[] = {
            "animations/Walking.glb",
            "animations/Start Walking.glb",
            "animations/Running.glb"
        };

        for (int i = 0; i < 3; i++)
        {
            if (!FileExists(fallbacks[i])) continue;
            worker->model = LoadModel(fallbacks[i]);
            if (worker->model.meshCount > 0) break;
            if (worker->model.materialCount > 0) UnloadModel(worker->model);
            memset(&worker->model, 0, sizeof(worker->model));
        }
    }

    if (worker->model.meshCount <= 0) return;

    worker->baseScale = 1.0f;

    if (worker->model.meshCount > 0)
    {
        BoundingBox bb = GetMeshBoundingBox(worker->model.meshes[0]);
        float h = bb.max.y - bb.min.y;
        if (h > 0.001f) worker->baseScale = 1.7f / h;
    }

    for (int i = 0; i < worker->model.materialCount; i++)
    {
        Texture2D t = worker->model.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture;
        if (t.id > 0) SetTextureFilter(t, TEXTURE_FILTER_BILINEAR);
    }

    LoadClip(worker, WORKER_ANIM_START_WALK, TextFormat("%s/Start Walking.glb", animDir));
    LoadClip(worker, WORKER_ANIM_WALK, TextFormat("%s/Walking.glb", animDir));
    LoadClip(worker, WORKER_ANIM_TURN, TextFormat("%s/Turning.glb", animDir));
    LoadClip(worker, WORKER_ANIM_TURN_RIGHT, TextFormat("%s/Right Turn.glb", animDir));
    LoadClip(worker, WORKER_ANIM_TURN_LEFT, TextFormat("%s/Left Turn.glb", animDir));
    LoadClip(worker, WORKER_ANIM_RUN, TextFormat("%s/Running.glb", animDir));

    worker->ready = true;
}

void WorkerSetShader(WorkerRig *worker, Shader shader)
{
    if (!worker->ready || shader.id <= 0) return;
    for (int i = 0; i < worker->model.materialCount; i++)
    {
        worker->model.materials[i].shader = shader;
    }
}

void WorkerUnload(WorkerRig *worker)
{
    if (!worker->ready) return;

    for (int i = 0; i < WORKER_ANIM_COUNT; i++)
    {
        if (worker->clips[i].anims && worker->clips[i].count > 0)
        {
            UnloadModelAnimations(worker->clips[i].anims, worker->clips[i].count);
        }
    }

    UnloadModel(worker->model);
    worker->ready = false;
}

int WorkerClipFrameCount(const WorkerRig *worker, WorkerAnim anim)
{
    if (!worker->ready) return 1;
    if (anim < 0 || anim >= WORKER_ANIM_COUNT) return 1;
    if (!worker->clips[anim].anims || worker->clips[anim].count <= 0) return 1;
    return worker->clips[anim].anims[0].frameCount;
}

void WorkerDrawAnimated(WorkerRig *worker, WorkerAnim anim, float frame, Vector3 position, float yawDegrees, float scale, Color tint)
{
    if (!worker->ready) return;

    if (anim >= 0 && anim < WORKER_ANIM_COUNT)
    {
        WorkerClip clip = worker->clips[anim];
        if (clip.anims && clip.count > 0)
        {
            int frameCount = clip.anims[0].frameCount;
            int frameIndex = (int)frame;
            if (frameCount > 1)
            {
                frameIndex %= frameCount;
                if (frameIndex < 0) frameIndex += frameCount;
            }
            else
            {
                frameIndex = 0;
            }

            if (IsModelAnimationValid(worker->model, clip.anims[0]))
            {
                UpdateModelAnimation(worker->model, clip.anims[0], frameIndex);
            }
        }
    }

    DrawModelEx(
        worker->model,
        position,
        (Vector3){ 0.0f, 1.0f, 0.0f },
        yawDegrees,
        (Vector3){ scale * worker->baseScale, scale * worker->baseScale, scale * worker->baseScale },
        tint
    );
}
