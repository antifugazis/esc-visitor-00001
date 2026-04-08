#include "player.h"

#include <math.h>
#include "raymath.h"

static float ClampPitch(float pitch)
{
    const float maxPitch = 1.45f;
    if (pitch > maxPitch) return maxPitch;
    if (pitch < -maxPitch) return -maxPitch;
    return pitch;
}

static Vector3 ForwardFromAngles(float yaw, float pitch)
{
    Vector3 f;
    f.x = cosf(pitch) * cosf(yaw);
    f.y = sinf(pitch);
    f.z = cosf(pitch) * sinf(yaw);
    return Vector3Normalize(f);
}

static Vector3 RightFromForward(Vector3 fwd)
{
    Vector3 right = { -fwd.z, 0.0f, fwd.x };
    return Vector3Normalize(right);
}

static float Snapf(float v, float step)
{
    return roundf(v / step) * step;
}

void PlayerInit(Player *player, Vector3 spawn)
{
    player->yaw = 0.0f;
    player->pitch = 0.0f;
    player->moveSpeed = 2.2f;
    player->sprintSpeed = 3.6f;
    player->lookSensitivity = 0.0017f;
    player->snapStep = 0.05f;
    player->bobPhase = 0.0f;
    player->movementAmount = 0.0f;
    player->velocity = (Vector3){ 0 };
    player->lockInput = false;

    player->camera.position = spawn;
    player->camera.target = Vector3Add(spawn, (Vector3){ 1.0f, 0.0f, 0.0f });
    player->camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    player->camera.fovy = 70.0f;
    player->camera.projection = CAMERA_PERSPECTIVE;

    DisableCursor();
}

void PlayerUpdate(Player *player, float dt, bool allowInput)
{
    if (!allowInput || player->lockInput)
    {
        Vector3 f = ForwardFromAngles(player->yaw, player->pitch);
        player->camera.target = Vector3Add(player->camera.position, f);
        player->movementAmount = 0.0f;
        return;
    }

    Vector2 mouseDelta = GetMouseDelta();
    player->yaw += mouseDelta.x * player->lookSensitivity;
    player->pitch -= mouseDelta.y * player->lookSensitivity;
    player->pitch = ClampPitch(player->pitch);

    Vector3 forward = ForwardFromAngles(player->yaw, player->pitch);
    Vector3 right = RightFromForward(forward);

    Vector3 wish = { 0 };
    if (IsKeyDown(KEY_W)) wish = Vector3Add(wish, forward);
    if (IsKeyDown(KEY_S)) wish = Vector3Subtract(wish, forward);
    if (IsKeyDown(KEY_D)) wish = Vector3Add(wish, right);
    if (IsKeyDown(KEY_A)) wish = Vector3Subtract(wish, right);
    wish.y = 0.0f;

    float targetSpeed = IsKeyDown(KEY_LEFT_SHIFT) ? player->sprintSpeed : player->moveSpeed;

    if (Vector3LengthSqr(wish) > 0.0001f)
    {
        wish = Vector3Normalize(wish);
        player->velocity = Vector3Scale(wish, targetSpeed);
    }
    else
    {
        player->velocity = Vector3Scale(player->velocity, 0.78f);
    }

    player->camera.position = Vector3Add(player->camera.position, Vector3Scale(player->velocity, dt));

    float speed = Vector3Length((Vector3){ player->velocity.x, 0.0f, player->velocity.z });
    player->movementAmount = speed;
    player->bobPhase += speed * dt * 7.0f;

    float bob = sinf(player->bobPhase) * 0.01f * (speed > 0.1f ? 1.0f : 0.0f);
    Vector3 camPos = player->camera.position;
    camPos.y += bob;

    player->camera.target = Vector3Add(camPos, forward);
    player->camera.position = camPos;
}

void PlayerUpdateNoclip(Player *player, float dt, bool allowInput)
{
    if (!allowInput || player->lockInput)
    {
        Vector3 f = ForwardFromAngles(player->yaw, player->pitch);
        player->camera.target = Vector3Add(player->camera.position, f);
        player->movementAmount = 0.0f;
        return;
    }

    Vector2 mouseDelta = GetMouseDelta();
    player->yaw += mouseDelta.x * player->lookSensitivity;
    player->pitch -= mouseDelta.y * player->lookSensitivity;
    player->pitch = ClampPitch(player->pitch);

    Vector3 forward = ForwardFromAngles(player->yaw, player->pitch);
    Vector3 right = RightFromForward(forward);
    Vector3 up = { 0.0f, 1.0f, 0.0f };

    Vector3 wish = { 0 };
    if (IsKeyDown(KEY_W)) wish = Vector3Add(wish, forward);
    if (IsKeyDown(KEY_S)) wish = Vector3Subtract(wish, forward);
    if (IsKeyDown(KEY_D)) wish = Vector3Add(wish, right);
    if (IsKeyDown(KEY_A)) wish = Vector3Subtract(wish, right);
    if (IsKeyDown(KEY_SPACE)) wish = Vector3Add(wish, up);
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) wish = Vector3Subtract(wish, up);

    float flySpeed = IsKeyDown(KEY_LEFT_SHIFT) ? 10.0f : 5.0f;
    if (Vector3LengthSqr(wish) > 0.0001f)
    {
        wish = Vector3Normalize(wish);
        player->velocity = Vector3Scale(wish, flySpeed);
    }
    else
    {
        player->velocity = (Vector3){ 0 };
    }

    player->camera.position = Vector3Add(player->camera.position, Vector3Scale(player->velocity, dt));
    player->movementAmount = Vector3Length(player->velocity);
    player->camera.target = Vector3Add(player->camera.position, forward);
}

void PlayerConstrain(Player *player, BoundingBox bounds)
{
    if (player->camera.position.x < bounds.min.x) player->camera.position.x = bounds.min.x;
    if (player->camera.position.y < bounds.min.y) player->camera.position.y = bounds.min.y;
    if (player->camera.position.z < bounds.min.z) player->camera.position.z = bounds.min.z;
    if (player->camera.position.x > bounds.max.x) player->camera.position.x = bounds.max.x;
    if (player->camera.position.y > bounds.max.y) player->camera.position.y = bounds.max.y;
    if (player->camera.position.z > bounds.max.z) player->camera.position.z = bounds.max.z;
}

Camera3D PlayerGetRenderCamera(const Player *player)
{
    Camera3D c = player->camera;

    Vector3 forward = ForwardFromAngles(player->yaw, player->pitch);
    c.position.x = Snapf(c.position.x, player->snapStep);
    c.position.y = Snapf(c.position.y, player->snapStep);
    c.position.z = Snapf(c.position.z, player->snapStep);
    c.target = Vector3Add(c.position, forward);

    return c;
}

float PlayerGetSpeed(const Player *player)
{
    return player->movementAmount;
}

float PlayerGetMoveAmount(const Player *player)
{
    return player->movementAmount;
}
