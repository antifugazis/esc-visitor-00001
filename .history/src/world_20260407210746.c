#include "world.h"

#include <stdio.h>
#include "rlgl.h"

void WorldInit(World *world, const char *officeGlbPath)
{
    world->hasOffice = false;
    world->hasPs2Shader = false;
    world->hasFogShader = false;
    world->hasLitShader = false;
    world->hasSky = false;
    world->hasLightmap = false;

    world->outsideBounds = (BoundingBox){
        .min = (Vector3){ -18.0f, 0.8f, -18.0f },
        .max = (Vector3){ 18.0f, 2.2f, 18.0f }
    };
    world->waitingRoomBounds = (BoundingBox){
        .min = (Vector3){ -4.0f, 0.9f, -4.0f },
        .max = (Vector3){ 6.5f, 2.0f, 5.0f }
    };

    world->outsideSpawn = (Vector3){ 0.0f, 1.7f, 11.0f };
    world->insideSpawn = (Vector3){ 0.5f, 1.55f, 2.8f };

    world->fogNear = 3.0f;
    world->fogFar = 12.0f;
    world->simplifyAmount = 0.0f;
    world->effectsEnabled = true;

    if (FileExists(officeGlbPath))
    {
        world->office = LoadModel(officeGlbPath);
        world->hasOffice = true;
        world->officeBounds = GetModelBoundingBox(world->office);

        BoundingBox bb = world->officeBounds;
        float centerX = (bb.min.x + bb.max.x) * 0.5f;
        float centerZ = (bb.min.z + bb.max.z) * 0.5f;
        float sizeX = bb.max.x - bb.min.x;
        float sizeZ = bb.max.z - bb.min.z;
        float eyeY = bb.min.y + 1.65f;

        float frontOffset = sizeZ * 0.18f;
        if (frontOffset < 4.0f) frontOffset = 4.0f;

        world->outsideSpawn = (Vector3){ centerX, eyeY, bb.max.z + frontOffset };
        world->insideSpawn = (Vector3){ centerX, eyeY, centerZ };

        world->outsideBounds = (BoundingBox){
            .min = (Vector3){ bb.min.x - sizeX * 0.7f, bb.min.y + 0.8f, bb.min.z - sizeZ * 0.7f },
            .max = (Vector3){ bb.max.x + sizeX * 0.7f, bb.min.y + 2.4f, bb.max.z + sizeZ * 0.7f }
        };

        world->waitingRoomBounds = (BoundingBox){
            .min = (Vector3){ centerX - 6.0f, bb.min.y + 0.9f, centerZ - 6.0f },
            .max = (Vector3){ centerX + 6.0f, bb.min.y + 2.2f, centerZ + 6.0f }
        };
    }

    world->ps2Shader = LoadShader(0, "src/shaders/ps2.fs");
    if (world->ps2Shader.id > 0) world->hasPs2Shader = true;

    world->litShader = LoadShader("src/shaders/lit.vs", "src/shaders/lit.fs");
    if (world->litShader.id > 0)
    {
        world->hasLitShader = true;

        float lightDir[3] = { -0.45f, -0.8f, -0.2f };
        float lightColor[3] = { 1.0f, 0.98f, 0.92f };
        float ambient[3] = { 0.22f, 0.24f, 0.27f };
        int useLightmap = 0;

        int dirLoc = GetShaderLocation(world->litShader, "lightDir");
        int colLoc = GetShaderLocation(world->litShader, "lightColor");
        int ambLoc = GetShaderLocation(world->litShader, "ambientColor");
        int lmLoc = GetShaderLocation(world->litShader, "useLightmap");

        SetShaderValue(world->litShader, dirLoc, lightDir, SHADER_UNIFORM_VEC3);
        SetShaderValue(world->litShader, colLoc, lightColor, SHADER_UNIFORM_VEC3);
        SetShaderValue(world->litShader, ambLoc, ambient, SHADER_UNIFORM_VEC3);
        SetShaderValue(world->litShader, lmLoc, &useLightmap, SHADER_UNIFORM_INT);

        // Keep office model on raylib default material shader for glTF compatibility
        // (glass/transparency/transmission often break with custom fragment shaders).
    }

    if (FileExists("assets/lightmap.png"))
    {
        world->lightmapTexture = LoadTexture("assets/lightmap.png");
        world->hasLightmap = world->lightmapTexture.id > 0;
    }

    if (world->hasLightmap && world->hasLitShader)
    {
        int lmSamplerLoc = GetShaderLocation(world->litShader, "lightmapTex");
        int useLightmap = 1;
        SetShaderValueTexture(world->litShader, lmSamplerLoc, world->lightmapTexture);
        SetShaderValue(world->litShader, GetShaderLocation(world->litShader, "useLightmap"), &useLightmap, SHADER_UNIFORM_INT);
    }

    if (FileExists("assets/sky.hdr"))
    {
        Image skyImg = LoadImage("assets/sky.hdr");
        if (skyImg.data)
        {
            world->skyTexture = LoadTextureFromImage(skyImg);
            UnloadImage(skyImg);
            world->hasSky = world->skyTexture.id > 0;
        }
    }

    world->skyShader = LoadShader("src/shaders/skydome.vs", "src/shaders/skydome.fs");
    if (world->hasSky && world->skyShader.id > 0)
    {
        Mesh m = GenMeshSphere(1.0f, 32, 32);
        world->skyDome = LoadModelFromMesh(m);
        world->skyDome.materials[0].shader = world->skyShader;
        world->skyDome.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = world->skyTexture;
        SetTextureFilter(world->skyTexture, TEXTURE_FILTER_BILINEAR);
    }

    world->fogShader = LoadShader(0, "src/shaders/fog.fs");
    if (world->fogShader.id > 0)
    {
        world->hasFogShader = true;
        int nearLoc = GetShaderLocation(world->fogShader, "fogNear");
        int farLoc = GetShaderLocation(world->fogShader, "fogFar");
        int fogColorLoc = GetShaderLocation(world->fogShader, "fogColor");

        SetShaderValue(world->fogShader, nearLoc, &world->fogNear, SHADER_UNIFORM_FLOAT);
        SetShaderValue(world->fogShader, farLoc, &world->fogFar, SHADER_UNIFORM_FLOAT);
        float fog[4] = { 0.16f, 0.14f, 0.13f, 1.0f };
        SetShaderValue(world->fogShader, fogColorLoc, fog, SHADER_UNIFORM_VEC4);
    }
}

void WorldUnload(World *world)
{
    if (world->hasSky) UnloadTexture(world->skyTexture);
    if (world->hasLightmap) UnloadTexture(world->lightmapTexture);
    if (world->hasSky && world->skyDome.meshCount > 0) UnloadModel(world->skyDome);
    if (world->skyShader.id > 0) UnloadShader(world->skyShader);
    if (world->litShader.id > 0) UnloadShader(world->litShader);
    if (world->hasOffice) UnloadModel(world->office);
    if (world->hasFogShader) UnloadShader(world->fogShader);
    if (world->hasPs2Shader) UnloadShader(world->ps2Shader);
}

void WorldUpdate(World *world, GameState state, float conditionNorm, bool irrelevantState)
{
    (void)state;
    world->simplifyAmount = irrelevantState ? 1.0f : (1.0f - conditionNorm) * 0.35f;

    if (world->hasFogShader)
    {
        float nearV = world->fogNear;
        float farV = world->fogFar - 2.0f * world->simplifyAmount;
        if (farV < nearV + 1.0f) farV = nearV + 1.0f;

        int nearLoc = GetShaderLocation(world->fogShader, "fogNear");
        int farLoc = GetShaderLocation(world->fogShader, "fogFar");
        SetShaderValue(world->fogShader, nearLoc, &nearV, SHADER_UNIFORM_FLOAT);
        SetShaderValue(world->fogShader, farLoc, &farV, SHADER_UNIFORM_FLOAT);
    }
}

void WorldDraw(World *world, const Camera3D *camera, GameState state)
{
    if (world->hasSky)
    {
        rlDisableBackfaceCulling();
        rlDisableDepthMask();
        DrawModel(world->skyDome, camera->position, 250.0f, WHITE);
        rlEnableDepthMask();
        rlEnableBackfaceCulling();
    }
    else if (state == GAME_STATE_OUTSIDE)
    {
        DrawSphere((Vector3){ camera->position.x, camera->position.y - 30.0f, camera->position.z }, 220.0f, (Color){ 164, 182, 210, 255 });
    }

    Color sun = (state == GAME_STATE_OUTSIDE)
        ? (Color){ 247, 236, 212, 255 }
        : (Color){ 35, 30, 27, 255 };

    DrawPlane((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector2){ 40.0f, 40.0f }, (Color){ 74, 66, 56, 255 });

    if (world->hasOffice)
    {
        rlDisableBackfaceCulling();
        BeginBlendMode(BLEND_ALPHA);
        if (world->effectsEnabled && world->hasPs2Shader)
        {
            BeginShaderMode(world->ps2Shader);
            DrawModel(world->office, (Vector3){ 0 }, 1.0f, WHITE);
            EndShaderMode();
        }
        else
        {
            DrawModel(world->office, (Vector3){ 0 }, 1.0f, WHITE);
        }
        EndBlendMode();
        rlEnableBackfaceCulling();
    }
    else
    {
        DrawCube((Vector3){ 0.0f, 1.5f, -1.0f }, 8.0f, 3.0f, 8.0f, (Color){ 80, 74, 68, 255 });
        DrawCube((Vector3){ 0.0f, 2.6f, -1.0f }, 8.0f, 0.2f, 8.0f, sun);
    }

    if (state == GAME_STATE_INSIDE)
    {
        DrawCube((Vector3){ 1.4f, 1.0f, 0.4f }, 0.8f, 0.8f, 0.8f, (Color){ 96, 92, 86, 255 });
        DrawCube((Vector3){ -1.1f, 1.0f, 0.2f }, 0.8f, 0.8f, 0.8f, (Color){ 96, 92, 86, 255 });
        DrawCube((Vector3){ -2.5f, 1.0f, 0.0f }, 1.0f, 2.0f, 0.2f, (Color){ 65, 58, 54, 255 });
    }
}

BoundingBox WorldGetActiveBounds(const World *world, GameState state)
{
    return (state == GAME_STATE_INSIDE) ? world->waitingRoomBounds : world->outsideBounds;
}

void WorldSetEffectsEnabled(World *world, bool enabled)
{
    world->effectsEnabled = enabled;
}

Shader WorldGetLitShader(const World *world)
{
    if (world->hasLitShader) return world->litShader;
    return (Shader){ 0 };
}

bool WorldHasSky(const World *world)
{
    return world->hasSky;
}

bool WorldHasLightmap(const World *world)
{
    return world->hasLightmap;
}
