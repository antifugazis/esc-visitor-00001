#ifndef WORLD_H
#define WORLD_H

#include "raylib.h"
#include "game.h"

typedef struct World {
    Model office;
    bool hasOffice;

    Shader ps2Shader;
    Shader fogShader;
    bool hasPs2Shader;
    bool hasFogShader;

    BoundingBox outsideBounds;
    BoundingBox waitingRoomBounds;

    Vector3 outsideSpawn;
    Vector3 insideSpawn;

    float fogNear;
    float fogFar;
    float simplifyAmount;
    bool effectsEnabled;
} World;

void WorldInit(World *world, const char *officeGlbPath);
void WorldUnload(World *world);
void WorldUpdate(World *world, GameState state, float conditionNorm, bool irrelevantState);
void WorldDraw(World *world, const Camera3D *camera, GameState state);
BoundingBox WorldGetActiveBounds(const World *world, GameState state);
void WorldSetEffectsEnabled(World *world, bool enabled);

#endif
