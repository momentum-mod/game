#pragma once

#include "cbase.h"

class CMomReplayBase;
class CMomRunStats;

struct HTTPRequestCompleted_t;
struct RunCompare_t;

class MomentumUtil
{
  public:

#ifdef CLIENT_DLL
    void UpdatePaintDecalScale(float fNewScale);
#endif

    bool GetColorFromHex(const char *hexColor, Color &into); // in hex color format RRGGBB or RRGGBBAA
    bool GetColorFromHex(uint32 HEX, Color &into); // in hex color format RRGGBBAA

    uint32 GetHexFromColor(const char *hexColor);
    uint32 GetHexFromColor(const Color &color);

    void GetHexStringFromColor(const Color &color, char *pBuffer, int maxLen);

    Color GetColorFromVariation(const float variation, float deadZone, const Color &normalcolor, const Color &increasecolor,
                                const Color &decreasecolor) const;
    // Formats time in ticks by a given tickrate into time. Includes minutes if time > minutes, hours if time > hours,
    // etc
    // Precision is miliseconds by default
    void FormatTime(float seconds, char *pOut, const int precision = 3, const bool fileName = false, const bool negativeTime = false) const;


    CMomReplayBase *GetBestTime(const char *szMapName, float tickrate, uint32 flags = 0) const;
    bool GetRunComparison(const char *szMapName, const float tickRate, const int flags, RunCompare_t *into) const;
    void FillRunComparison(const char *compareName, CMomRunStats *kvBestRun, RunCompare_t *into) const;

    // Checks if source is within a rectangle formed by leftCorner and rightCorner
    bool IsInBounds(const Vector2D &source, const Vector2D &bottomLeft, const Vector2D &topRight) const
    {
        return (source.x > bottomLeft.x && source.x < topRight.x) && (source.y > bottomLeft.y && source.y < topRight.y);
    }

    bool IsInBounds(const int x, const int y, const int rectX, const int rectY, const int rectW, const int rectH) const
    {
        return IsInBounds(Vector2D(x, y), Vector2D(rectX, rectY), Vector2D(rectX + rectW, rectY + rectH));
    }

    void KVSaveVector(KeyValues *kvInto, const char *pName, const Vector &toSave);
    void KVLoadVector(KeyValues *kvFrom, const char *pName, Vector &vecInto);

    void KVSaveQAngles(KeyValues *kvInto, const char *pName, const QAngle &toSave);
    void KVLoadQAngles(KeyValues *kvFrom, const char *pName, QAngle &angInto);

    void KnifeTrace(const Vector &vecShootPos, const QAngle &lookAng, bool bStab, CBaseEntity *pAttacker, CBaseEntity *pSoundSource, trace_t *trOutput, Vector *vForwardOut);
    void KnifeSmack(const trace_t &tr_in, CBaseEntity *pSoundSource, const QAngle &lookAng, const bool bStab);
};

extern MomentumUtil *g_pMomentumUtil;