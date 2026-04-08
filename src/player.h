#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

typedef struct Player {
    Camera3D camera;
    float yaw;
    float pitch;
    float moveSpeed;
    float sprintSpeed;
    float lookSensitivity;
    float snapStep;
    float bobPhase;
    float movementAmount;
    Vector3 velocity;
    bool lockInput;
} Player;

void PlayerInit(Player *player, Vector3 spawn);
void PlayerUpdate(Player *player, float dt, bool allowInput);
void PlayerUpdateNoclip(Player *player, float dt, bool allowInput);
void PlayerConstrain(Player *player, BoundingBox bounds);
Camera3D PlayerGetRenderCamera(const Player *player);
float PlayerGetSpeed(const Player *player);
float PlayerGetMoveAmount(const Player *player);

#endif
