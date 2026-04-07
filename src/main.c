#include "raylib.h"

#include "audio.h"
#include "condition.h"
#include "game.h"
#include "meta.h"
#include "mirror.h"
#include "npcs.h"
#include "player.h"
#include "tone.h"
#include "world.h"

#include <string.h>

int main(void)
{
    InitWindow(GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT, "VISITOR #00001");
    SetTargetFPS(30);

    InitAudioDevice();

    GameState state = GAME_STATE_OUTSIDE;
    GameState prevState = state;

    World world;
    WorldInit(&world, "office_building.glb");

    Player player;
    PlayerInit(&player, world.outsideSpawn);

    NpcSystem npcs;
    NpcsInit(&npcs);

    ConditionSystem condition;
    ConditionInit(&condition);

    ToneSystem tone;
    ToneInit(&tone);

    AudioSystem audio;
    AudioSystemInit(&audio);

    MirrorSystem mirror;
    MirrorInit(&mirror, GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT);

    MetaSystem meta;
    MetaInit(&meta, "visitor.dat");

    bool itemInInventory = false;
    char itemDescription[128] = { 0 };
    MetaGetItemDescription(&meta, itemDescription, (int)sizeof(itemDescription));

    int outsideVisits = 0;
    float footstepAccumulator = 0.0f;
    WorldSetEffectsEnabled(&world, false);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        if (IsKeyPressed(KEY_F4) && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
        {
            MetaMarkAltF4Attempt(&meta);
        }

        if (state == GAME_STATE_OUTSIDE && IsKeyPressed(KEY_E))
        {
            state = GAME_STATE_INSIDE;
            player.camera.position = world.insideSpawn;
            NpcsMarkInsideVisited(&npcs);
        }
        else if (state == GAME_STATE_INSIDE && IsKeyPressed(KEY_Q))
        {
            state = GAME_STATE_OUTSIDE;
            player.camera.position = world.outsideSpawn;
            outsideVisits += 1;
            NpcsSetOutsideVisitCount(&npcs, outsideVisits);
        }

        if (prevState != state) prevState = state;

        PlayerUpdate(&player, dt, true);
        PlayerConstrain(&player, WorldGetActiveBounds(&world, state));

        float speed = PlayerGetSpeed(&player);
        float stillDt = speed < 0.02f ? dt : 0.0f;

        ConditionUpdate(&condition, dt, speed);
        ToneUpdate(&tone, dt, speed);
        MetaUpdate(&meta, dt, stillDt);

        bool irrelevant = ConditionIsIrrelevant(&condition);
        WorldUpdate(&world, state, ConditionGetNormalized(&condition), irrelevant);

        if (state == GAME_STATE_OUTSIDE)
        {
            NpcsUpdate(&npcs, dt, state, player.camera.position);
            if (!itemInInventory && NpcsConsumeItemGrant(&npcs)) itemInInventory = true;
        }

        MirrorUpdate(&mirror, dt, &player.camera, meta.data.totalPlaytimeSeconds + meta.sessionPlaytime);

        footstepAccumulator += speed * dt;
        if (footstepAccumulator > 1.7f)
        {
            AudioSystemPlayFootstep(&audio);
            footstepAccumulator = 0.0f;
        }

        AudioSystemUpdate(&audio, dt, state == GAME_STATE_INSIDE, ConditionGetNormalized(&condition));

        Camera3D renderCam = PlayerGetRenderCamera(&player);

        if (state == GAME_STATE_INSIDE)
        {
            Camera3D mirrorCam = MirrorGetReflectionCamera(&mirror, &renderCam);
            MirrorBeginReflection(&mirror);
            BeginMode3D(mirrorCam);
                WorldDraw(&world, &mirrorCam, state);
                NpcsDraw(&npcs, state);
            EndMode3D();
            MirrorEndReflection();
        }

        BeginDrawing();
            if (state == GAME_STATE_OUTSIDE)
                ClearBackground((Color){ 214, 214, 214, 255 });
            else
                ClearBackground((Color){ 28, 28, 28, 255 });

            BeginMode3D(renderCam);
                WorldDraw(&world, &renderCam, state);
                NpcsDraw(&npcs, state);
                if (state == GAME_STATE_INSIDE) MirrorDraw(&mirror);
            EndMode3D();

            if (MetaHasWelcomeBack(&meta) && meta.sessionPlaytime < 6.0f)
            {
                DrawText(TextFormat("WELCOME BACK, %s", MetaGetUsername(&meta)), 12, 12, 20, (Color){ 220, 220, 220, 255 });
            }

            if (MetaShouldShowStillThere(&meta))
            {
                DrawText(TextFormat("> %s ARE YOU STILL THERE", MetaGetUsername(&meta)), 12, 40, 16, (Color){ 210, 210, 210, 220 });
            }

            if (itemInInventory)
            {
                DrawText("ITEM ACQUIRED", 12, 66, 16, (Color){ 224, 224, 224, 255 });
                DrawText(itemDescription, 12, 86, 16, (Color){ 196, 196, 196, 255 });
            }

            DrawText((state == GAME_STATE_OUTSIDE) ? "E: ENTER BUILDING" : "Q: RETURN OUTSIDE", 12, GAME_WINDOW_HEIGHT - 50, 16, (Color){ 186, 186, 186, 255 });
            ConditionDraw(&condition, GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT);
        EndDrawing();
    }

    MetaShutdown(&meta, "visitor.dat");

    MirrorUnload(&mirror);
    AudioSystemUnload(&audio);
    ToneUnload(&tone);
    WorldUnload(&world);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
