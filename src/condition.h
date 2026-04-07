#ifndef CONDITION_H
#define CONDITION_H

#include "raylib.h"

typedef enum ConditionState {
    CONDITION_FULL = 0,
    CONDITION_LOW,
    CONDITION_CRITICAL,
    CONDITION_IRRELEVANT
} ConditionState;

typedef struct ConditionSystem {
    float maxValue;
    float value;
    float stillSeconds;
    ConditionState state;
} ConditionSystem;

void ConditionInit(ConditionSystem *condition);
void ConditionUpdate(ConditionSystem *condition, float dt, float playerSpeed);
void ConditionDraw(const ConditionSystem *condition, int internalWidth, int internalHeight);
float ConditionGetNormalized(const ConditionSystem *condition);
bool ConditionIsIrrelevant(const ConditionSystem *condition);

#endif
