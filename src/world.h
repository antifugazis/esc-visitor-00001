#ifndef WORLD_H
#define WORLD_H

#include "raylib.h"
#include "game.h"

typedef struct World {
    Model office;
    bool hasOffice;
    BoundingBox officeBounds;

    Shader ps2Shader;
    Shader fogShader;
    Shader litShader;
    Shader skyShader;
    bool hasPs2Shader;
    bool hasFogShader;
    bool hasLitShader;
    bool hasSky;
    bool hasLightmap;

    Model skyDome;
    Texture2D skyTexture;
    Texture2D lightmapTexture;

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
Shader WorldGetLitShader(const World *world);
bool WorldHasSky(const World *world);
bool WorldHasLightmap(const World *world);

#endif
