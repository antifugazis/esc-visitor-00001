#include "npcs.h"

#include <math.h>
#include "raymath.h"

void NpcsInit(NpcSystem *npcs)
{
    npcs->count = 14;
    npcs->outsideTimer = 0.0f;
    npcs->freezeTriggered = false;
    npcs->freezeTurnTimer = 0.0f;
    npcs->itemGranted = false;
    npcs->insideVisited = false;
    npcs->outsideVisits = 0;

    for (int i = 0; i < npcs->count; i++)
    {
        float lane = (float)((i % 5) - 2) * 1.2f;
        float zOffset = (float)(i / 5) * 3.1f;
        npcs->list[i].position = (Vector3){ lane, 1.0f, 15.0f + zOffset };
        npcs->list[i].heading = Vector3Normalize((Vector3){ 0.0f, 0.0f, -1.0f });
        npcs->list[i].speed = 1.0f + 0.1f * (float)(i % 3);
        npcs->list[i].headTurn = 0.0f;
        npcs->list[i].active = true;
    }
}

void NpcsSetOutsideVisitCount(NpcSystem *npcs, int outsideVisits)
{
    npcs->outsideVisits = outsideVisits;

    int shouldRemain = 14 - outsideVisits;
    if (shouldRemain < 0) shouldRemain = 0;

    for (int i = 0; i < npcs->count; i++)
    {
        npcs->list[i].active = (i < shouldRemain);
        if (npcs->list[i].active)
        {
            float lane = (float)((i % 5) - 2) * 1.2f;
            float zOffset = (float)(i / 5) * 3.1f;
            npcs->list[i].position = (Vector3){ lane, 1.0f, 15.0f + zOffset };
        }
    }

    if (!npcs->insideVisited)
    {
        npcs->freezeTriggered = false;
        npcs->freezeTurnTimer = 0.0f;
        npcs->outsideTimer = 0.0f;
    }
}

void NpcsMarkInsideVisited(NpcSystem *npcs)
{
    npcs->insideVisited = true;
}

void NpcsUpdate(NpcSystem *npcs, float dt, GameState state, Vector3 playerPos)
{
    if (state != GAME_STATE_OUTSIDE) return;

    npcs->outsideTimer += dt;

    if (!npcs->freezeTriggered && npcs->outsideTimer >= 30.0f)
    {
        npcs->freezeTriggered = true;
        npcs->freezeTurnTimer = 0.0f;
    }

    if (npcs->freezeTriggered)
    {
        npcs->freezeTurnTimer += dt;
        float t = npcs->freezeTurnTimer / 2.0f;
        if (t > 1.0f) t = 1.0f;

        for (int i = 0; i < npcs->count; i++)
        {
            if (!npcs->list[i].active) continue;
            npcs->list[i].headTurn = t;
        }
        return;
    }

    for (int i = 0; i < npcs->count; i++)
    {
        if (!npcs->list[i].active) continue;

        npcs->list[i].position = Vector3Add(npcs->list[i].position, Vector3Scale(npcs->list[i].heading, npcs->list[i].speed * dt));
        if (npcs->list[i].position.z < -5.0f) npcs->list[i].position.z = 20.0f + (float)(i % 4) * 2.0f;

        if (!npcs->itemGranted)
        {
            float d = Vector3Distance(npcs->list[i].position, playerPos);
            if (d < 1.2f)
            {
                npcs->itemGranted = true;
            }
        }
    }
}

void NpcsDraw(const NpcSystem *npcs, GameState state)
{
    if (state != GAME_STATE_OUTSIDE) return;

    for (int i = 0; i < npcs->count; i++)
    {
        if (!npcs->list[i].active) continue;

        Color c = npcs->freezeTriggered ? (Color){ 132, 122, 112, 255 } : (Color){ 112, 106, 100, 255 };
        Vector3 p = npcs->list[i].position;

        DrawCube((Vector3){ p.x, p.y, p.z }, 0.55f, 1.6f, 0.32f, c);

        Vector3 head = { p.x, p.y + 1.0f, p.z };
        float look = npcs->list[i].headTurn;
        head.x += look * 0.06f;
        head.z -= look * 0.06f;
        DrawCube(head, 0.30f, 0.30f, 0.30f, (Color){ 98, 92, 86, 255 });
    }
}

bool NpcsConsumeItemGrant(NpcSystem *npcs)
{
    if (npcs->itemGranted)
    {
        npcs->itemGranted = false;
        return true;
    }
    return false;
}
