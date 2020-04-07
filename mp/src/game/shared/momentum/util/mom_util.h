#pragma once

#include "run/run_stats.h"

class CMomReplayBase;
struct RunCompare_t;

namespace MomUtil
{
#ifdef CLIENT_DLL
    void UpdatePaintDecalScale(float fNewScale);

    // If calling commands through the engine is not possible (e.g. calling server command before it's been spawned), this does the trick
    // Searches for the command and invokes it's associated function directly
    void DispatchConCommand(const char *pszCommand);
#endif

    // Mounts CS:S, TF2, etc content
    void MountGameFiles();

    bool GetColorFromHex(const char *hexColor, Color &into); // in hex color format RRGGBB or RRGGBBAA
    bool GetColorFromHex(uint32 HEX, Color &into); // in hex color format RRGGBBAA

    uint32 GetHexFromColor(const char *hexColor);
    uint32 GetHexFromColor(const Color &color);

    void GetHexStringFromColor(const Color &color, char *pBuffer, int maxLen);

    Color GetColorFromVariation(const float variation, float deadZone, const Color &normalcolor, const Color &increasecolor,
                                const Color &decreasecolor);
    Color ColorLerp(float prog, const Color& A, const Color& B);
    // Formats time in ticks by a given tickrate into time. Includes minutes if time > minutes, hours if time > hours,
    // etc
    // Precision is miliseconds by default
    void FormatTime(float seconds, char *pOut, const int precision = 3, const bool fileName = false,
                    const bool negativeTime = false);

    // Taking an input time_t, formats the time difference between now and the given time, up to years of difference.
    // Example output: "5 minutes ago"
    // Returns true if worked, else false if bad input
    bool GetTimeAgoString(time_t *input, char *pOut, size_t outLen);
    bool GetTimeAgoString(const char *pISODate, char *pOut, size_t outLen);

    // Converts an ISO-8601 date string to a time_t
    bool ISODateToTimeT(const char *pISODate, time_t *out);

    CMomReplayBase *GetBestTime(const char *szMapName, float tickrate, int trackNumber, uint32 flags = 0);
    bool GetRunComparison(const char *szMapName, const float tickRate, const int trackNumber, const int flags, RunCompare_t *into);
    void FillRunComparison(const char *compareName, CMomRunStats *kvBestRun, RunCompare_t *into);

    // Checks if source is within a rectangle formed by leftCorner and rightCorner
    bool IsInBounds(const Vector2D &source, const Vector2D &bottomLeft, const Vector2D &topRight);
    bool IsInBounds(const int x, const int y, const int rectX, const int rectY, const int rectW, const int rectH);

    void KVSaveVector(KeyValues *kvInto, const char *pName, const Vector &toSave);
    void KVLoadVector(KeyValues *kvFrom, const char *pName, Vector &vecInto);

    void KVSaveQAngles(KeyValues *kvInto, const char *pName, const QAngle &toSave);
    void KVLoadQAngles(KeyValues *kvFrom, const char *pName, QAngle &angInto);

    bool GetSHA1Hash(const CUtlBuffer &buf, char *pOut, size_t outLen);
    bool GetFileHash(char *pOut, size_t outLen, const char *pFileName, const char *pPath = "GAME");
    // Check to see if a file exists via a known hash for it. Handles reading the file and getting its hash.
    bool FileExists(const char *pFileName, const char *pFileHash, const char *pPath = "GAME");
    bool MapThumbnailExists(const char *pMapName);
};