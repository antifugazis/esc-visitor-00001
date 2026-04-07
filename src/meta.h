#ifndef META_H
#define META_H

#include <stdbool.h>

typedef struct VisitorSaveData {
    unsigned int magic;
    unsigned int version;

    float totalPlaytimeSeconds;
    unsigned int sessions;
    unsigned int corridorsVisitedMask;
    float totalStillSeconds;
    unsigned int triedAltF4;

    float lastSessionStillSeconds;
    float lastSessionPlaytimeSeconds;
    unsigned int lastSessionTriedAltF4;
} VisitorSaveData;

typedef struct MetaSystem {
    VisitorSaveData data;
    bool hadPriorSave;

    char username[64];

    float sessionPlaytime;
    float sessionStillSeconds;
    bool sessionTriedAltF4;
} MetaSystem;

void MetaInit(MetaSystem *meta, const char *savePath);
void MetaUpdate(MetaSystem *meta, float dt, float stillSecondsThisFrame);
void MetaMarkCorridor(MetaSystem *meta, unsigned int corridorBit);
void MetaMarkAltF4Attempt(MetaSystem *meta);
void MetaShutdown(MetaSystem *meta, const char *savePath);

const char *MetaGetUsername(const MetaSystem *meta);
bool MetaHasWelcomeBack(const MetaSystem *meta);
bool MetaShouldShowStillThere(const MetaSystem *meta);
void MetaGetItemDescription(const MetaSystem *meta, char *buffer, int bufferSize);

#endif
