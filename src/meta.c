#include "meta.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VISITOR_MAGIC 0x56535452u
#define VISITOR_VERSION 1u

static bool LoadSave(const char *savePath, VisitorSaveData *outData)
{
    FILE *f = fopen(savePath, "rb");
    if (!f) return false;

    VisitorSaveData data = { 0 };
    size_t n = fread(&data, sizeof(VisitorSaveData), 1, f);
    fclose(f);

    if (n != 1) return false;
    if (data.magic != VISITOR_MAGIC || data.version != VISITOR_VERSION) return false;

    *outData = data;
    return true;
}

static void SaveToDisk(const char *savePath, const VisitorSaveData *data)
{
    FILE *f = fopen(savePath, "wb");
    if (!f) return;
    fwrite(data, sizeof(VisitorSaveData), 1, f);
    fclose(f);
}

void MetaInit(MetaSystem *meta, const char *savePath)
{
    memset(meta, 0, sizeof(*meta));

#ifdef _WIN32
    const char *user = getenv("USERNAME");
#else
    const char *user = getenv("USER");
#endif

    if (!user || !user[0]) user = "VISITOR";
    strncpy(meta->username, user, sizeof(meta->username) - 1);
    meta->username[sizeof(meta->username) - 1] = '\0';

    VisitorSaveData loaded = { 0 };
    if (LoadSave(savePath, &loaded))
    {
        meta->data = loaded;
        meta->hadPriorSave = true;
    }
    else
    {
        meta->data.magic = VISITOR_MAGIC;
        meta->data.version = VISITOR_VERSION;
        meta->hadPriorSave = false;
    }

    meta->data.sessions += 1;
}

void MetaUpdate(MetaSystem *meta, float dt, float stillSecondsThisFrame)
{
    meta->sessionPlaytime += dt;
    meta->sessionStillSeconds += stillSecondsThisFrame;
}

void MetaMarkCorridor(MetaSystem *meta, unsigned int corridorBit)
{
    meta->data.corridorsVisitedMask |= corridorBit;
}

void MetaMarkAltF4Attempt(MetaSystem *meta)
{
    meta->sessionTriedAltF4 = true;
}

void MetaShutdown(MetaSystem *meta, const char *savePath)
{
    meta->data.totalPlaytimeSeconds += meta->sessionPlaytime;
    meta->data.totalStillSeconds += meta->sessionStillSeconds;
    if (meta->sessionTriedAltF4) meta->data.triedAltF4 += 1;

    meta->data.lastSessionStillSeconds = meta->sessionStillSeconds;
    meta->data.lastSessionPlaytimeSeconds = meta->sessionPlaytime;
    meta->data.lastSessionTriedAltF4 = meta->sessionTriedAltF4 ? 1u : 0u;

    SaveToDisk(savePath, &meta->data);
}

const char *MetaGetUsername(const MetaSystem *meta)
{
    return meta->username;
}

bool MetaHasWelcomeBack(const MetaSystem *meta)
{
    return meta->hadPriorSave;
}

bool MetaShouldShowStillThere(const MetaSystem *meta)
{
    return meta->sessionStillSeconds >= 300.0f;
}

void MetaGetItemDescription(const MetaSystem *meta, char *buffer, int bufferSize)
{
    if (meta->data.lastSessionStillSeconds >= 180.0f)
    {
        snprintf(buffer, (size_t)bufferSize, "Unlabeled keycard. Warm from being held.");
        return;
    }

    if (meta->data.lastSessionTriedAltF4)
    {
        snprintf(buffer, (size_t)bufferSize, "Transit pass. Corner bent inward.");
        return;
    }

    if (meta->data.corridorsVisitedMask != 0)
    {
        snprintf(buffer, (size_t)bufferSize, "Visitor badge. Ink offset by a millimeter.");
        return;
    }

    snprintf(buffer, (size_t)bufferSize, "Plastic token. No markings, no weight.");
}
