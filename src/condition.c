#include "condition.h"

#include <stdio.h>

void ConditionInit(ConditionSystem *condition)
{
    condition->maxValue = 100.0f;
    condition->value = 100.0f;
    condition->stillSeconds = 0.0f;
    condition->state = CONDITION_FULL;
}

void ConditionUpdate(ConditionSystem *condition, float dt, float playerSpeed)
{
    const float baseDrain = condition->maxValue / (15.0f * 60.0f);
    float drain = baseDrain;

    if (playerSpeed < 0.02f)
    {
        condition->stillSeconds += dt;
        drain += baseDrain * 1.45f;
    }

    condition->value -= drain * dt;
    if (condition->value < 0.0f) condition->value = 0.0f;

    float n = condition->value / condition->maxValue;
    if (n <= 0.0f) condition->state = CONDITION_IRRELEVANT;
    else if (n <= 0.20f) condition->state = CONDITION_CRITICAL;
    else if (n <= 0.45f) condition->state = CONDITION_LOW;
    else condition->state = CONDITION_FULL;
}

void ConditionDraw(const ConditionSystem *condition, int internalWidth, int internalHeight)
{
    if (condition->state == CONDITION_IRRELEVANT) return;

    int blocks = 8;
    float n = condition->value / condition->maxValue;
    int filled = (int)(n * blocks + 0.5f);
    if (filled < 0) filled = 0;
    if (filled > blocks) filled = blocks;

    char meter[32] = { 0 };
    for (int i = 0; i < blocks; i++) meter[i] = (i < filled) ? '#' : '-';
    meter[blocks] = '\0';

    const char *label = "FULL";
    if (condition->state == CONDITION_LOW) label = "LOW";
    if (condition->state == CONDITION_CRITICAL) label = "CRITICAL";

    DrawText(TextFormat("CONDITION: %s", meter), 8, internalHeight - 24, 10, (Color){ 190, 224, 170, 255 });
    DrawText(TextFormat("STATE: %s", label), 8, internalHeight - 13, 8, (Color){ 160, 186, 146, 255 });
    DrawRectangleLines(6, internalHeight - 28, internalWidth - 12, 24, (Color){ 90, 116, 84, 255 });
}

float ConditionGetNormalized(const ConditionSystem *condition)
{
    return condition->value / condition->maxValue;
}

bool ConditionIsIrrelevant(const ConditionSystem *condition)
{
    return condition->state == CONDITION_IRRELEVANT;
}
