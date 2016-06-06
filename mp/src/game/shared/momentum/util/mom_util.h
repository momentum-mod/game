#ifndef MOM_UTIL_H
#define MOM_UTIL_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "KeyValues.h"
#include "steam/steam_api.h"
#include "run_compare.h"
#include "UtlSortVector.h"
#include "filesystem.h"
#include "gason.h"

class MomentumUtil
{
public:
    void PostTimeCallback(HTTPRequestCompleted_t*, bool);
    void DownloadCallback(HTTPRequestCompleted_t*, bool);
    void PostTime(const char* URL);
    void DownloadMap(const char*);

    void CreateAndSendHTTPReq(const char*, CCallResult<MomentumUtil, HTTPRequestCompleted_t>*,
        CCallResult<MomentumUtil, HTTPRequestCompleted_t>::func_t);

    CCallResult<MomentumUtil, HTTPRequestCompleted_t> cbDownloadCallback;
    CCallResult<MomentumUtil, HTTPRequestCompleted_t> cbPostTimeCallback;

    void GetRemoteRepoModVersion();

    CCallResult<MomentumUtil, HTTPRequestCompleted_t> cbVersionCallback;
    void VersionCallback(HTTPRequestCompleted_t*, bool);

    //Color GetColorFromVariation(float variation, float deadZone, Color normalcolor, Color increasecolor, Color decreasecolor);
    Color* GetColorFromHex(const char* hexColor); //in hex color format RRGGBB
    Color m_newColor;
    
    Color GetColorFromVariation(float variation, float deadZone, Color normalcolor, Color increasecolor, Color decreasecolor) const;
    //Formats time in ticks by a given tickrate into time. Includes minutes if time > minutes, hours if time > hours, etc
    //Precision is miliseconds by default
    void FormatTime(float seconds, char *pOut, int precision = 3, bool fileName = false) const;

    KeyValues *GetBestTime(KeyValues *kvInput, const char *szMapName, float tickrate, int flags = 0);
    bool GetRunComparison(const char *szMapName, float tickRate, int flags, RunCompare_t *into);

    bool FloatEquals(float a, float b, float epsilon = FLT_EPSILON) const
    {
        return fabs(a - b) < epsilon;
    }

#ifdef GAME_DLL

    void DispatchTimerStateMessage(CBasePlayer *, int, bool) const;

#endif
};

class CTimeSortFunc
{
public:
    bool Less(KeyValues* lhs, KeyValues* rhs, void *)
    {
        return (Q_atof(lhs->GetName())) < Q_atof(rhs->GetName());
    }
};

extern MomentumUtil *mom_UTIL;

#endif //MOM_UTIL_H