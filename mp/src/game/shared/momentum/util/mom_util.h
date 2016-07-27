#ifndef MOM_UTIL_H
#define MOM_UTIL_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "KeyValues.h"
#include "steam/steam_api.h"
#include "run_compare.h"
#include "run_stats.h"
#include "UtlSortVector.h"
#include "filesystem.h"
#include "gason.h"
#ifdef CLIENT_DLL
#include "ChangelogPanel.h"
#endif

class MomentumUtil
{
public:
    void DownloadCallback(HTTPRequestCompleted_t*, bool);

    void DownloadMap(const char*);

    void CreateAndSendHTTPReq(const char*, CCallResult<MomentumUtil, HTTPRequestCompleted_t>*,
        CCallResult<MomentumUtil, HTTPRequestCompleted_t>::func_t);

    bool CreateAndSendHTTPReqWithPost(const char*, CCallResult<MomentumUtil, HTTPRequestCompleted_t>*,
        CCallResult<MomentumUtil, HTTPRequestCompleted_t>::func_t, KeyValues *params);

    CCallResult<MomentumUtil, HTTPRequestCompleted_t> cbDownloadCallback;

#ifdef CLIENT_DLL
    void GetRemoteChangelog();
    CCallResult<MomentumUtil, HTTPRequestCompleted_t> cbChangeLog;
    void ChangelogCallback(HTTPRequestCompleted_t*, bool);

    void GetRemoteRepoModVersion();
    CCallResult<MomentumUtil, HTTPRequestCompleted_t> cbVersionCallback;
    void VersionCallback(HTTPRequestCompleted_t*, bool);

    //For the ComparisonsSettingsPage
    void GenerateBogusComparison(KeyValues *kvOut);
    void GenerateBogusRunStats(C_MomRunStats *pStatsOut);
#endif

    //Color GetColorFromVariation(float variation, float deadZone, Color normalcolor, Color increasecolor, Color decreasecolor);
    Color* GetColorFromHex(const char* hexColor); //in hex color format RRGGBB
    Color m_newColor;
    
    Color GetColorFromVariation(float variation, float deadZone, Color normalcolor, Color increasecolor, Color decreasecolor) const;
    //Formats time in ticks by a given tickrate into time. Includes minutes if time > minutes, hours if time > hours, etc
    //Precision is miliseconds by default
    void FormatTime(float seconds, char *pOut, int precision = 3, bool fileName = false) const;

    KeyValues *GetBestTime(KeyValues *kvInput, const char *szMapName, float tickrate, int flags = 0);
    bool GetRunComparison(const char *szMapName, float tickRate, int flags, RunCompare_t *into);
    void FillRunComparison(const char *compareName, KeyValues *kvBestRun, RunCompare_t *into);
    

    bool FloatEquals(float a, float b, float epsilon = FLT_EPSILON) const
    {
        return fabs(a - b) < epsilon;
    }
    
    //Checks if source is within a rectangle formed by leftCorner and rightCorner
    bool IsInBounds(Vector2D source, Vector2D bottomLeft, Vector2D topRight) const
    {
        return (source.x > bottomLeft.x && source.x < topRight.x) &&
            (source.y > bottomLeft.y && source.y < topRight.y);
    }

    bool IsInBounds(int x, int y, int rectX, int rectY, int rectW, int rectH) const
    {
        return IsInBounds(Vector2D(x, y), Vector2D(rectX, rectY),
            Vector2D(rectX + rectW, rectY + rectH));
    }

    void KVSaveVector(KeyValues *kvInto, const char *pName, Vector &toSave);
    void KVLoadVector(KeyValues *kvFrom, const char *pName, Vector &vecInto);

    void KVSaveQAngles(KeyValues *kvInto, const char *pName, QAngle &toSave);
    void KVLoadQAngles(KeyValues *kvFrom, const char *pName, QAngle &angInto);
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