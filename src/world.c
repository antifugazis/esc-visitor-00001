#include "world.h"

#include <stdio.h>

void WorldInit(World *world, const char *officeGlbPath)
{
    world->hasOffice = false;
    world->hasPs2Shader = false;
    world->hasFogShader = false;

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
    }

    world->ps2Shader = LoadShader(0, "src/shaders/ps2.fs");
    if (world->ps2Shader.id > 0) world->hasPs2Shader = true;

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
    Color sun = (state == GAME_STATE_OUTSIDE)
        ? (Color){ 247, 236, 212, 255 }
        : (Color){ 35, 30, 27, 255 };

    DrawPlane((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector2){ 40.0f, 40.0f }, (Color){ 74, 66, 56, 255 });

    if (world->hasOffice)
    {
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
