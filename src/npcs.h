#ifndef NPCS_H
#define NPCS_H

#include "raylib.h"
#include "game.h"
#include "worker.h"

typedef struct Npc {
    Vector3 position;
    Vector3 heading;
    float speed;
    float headTurn;
    float yawDegrees;
    float animFrame;
    float startWalkTimer;
    bool active;
} Npc;

typedef struct NpcSystem {
    Npc list[15];
    int count;

    float outsideTimer;
    bool freezeTriggered;
    float freezeTurnTimer;

    bool itemGranted;
    bool insideVisited;
    int outsideVisits;
} NpcSystem;

void NpcsInit(NpcSystem *npcs);
void NpcsSetOutsideVisitCount(NpcSystem *npcs, int outsideVisits);
void NpcsMarkInsideVisited(NpcSystem *npcs);
void NpcsUpdate(NpcSystem *npcs, float dt, GameState state, Vector3 playerPos);
void NpcsDraw(NpcSystem *npcs, WorkerRig *worker, GameState state, float dt);
bool NpcsConsumeItemGrant(NpcSystem *npcs);

#endif
