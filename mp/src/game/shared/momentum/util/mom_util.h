#ifndef MOM_UTIL_H
#define MOM_UTIL_H
#ifdef _WIN32
#pragma once
#endif

#include "filesystem.h"
#include "gason.h"

class MomentumUtil
{
public:

#ifdef GAME_DLL
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
        
#endif

    Color GetColorFromVariation(float variation, float deadZone, Color normalcolor, Color increasecolor, Color decreasecolor);

    //Formats time in ticks by a given tickrate into time. Includes minutes if time > minutes, hours if time > hours, etc
    //Precision is miliseconds by default
    void FormatTime(float ticks, float rate, char *pOut, int precision = 3);
};

extern MomentumUtil mom_UTIL;

#endif //MOM_UTIL_H