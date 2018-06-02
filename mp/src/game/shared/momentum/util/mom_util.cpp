#include "cbase.h"

#include "filesystem.h"
#include "mom_util.h"
#include "momentum/mom_shareddefs.h"
#include "run/mom_replay_factory.h"
#include "run/mom_replay_base.h"
#include <gason.h>
#include "run/run_compare.h"
#include "run/run_stats.h"
#include "effect_dispatch_data.h"
#ifdef CLIENT_DLL
#include "ChangelogPanel.h"
#include "materialsystem/imaterialvar.h"
#endif

#include "steam/steam_api.h"

#include "tier0/memdbgon.h"

extern IFileSystem *filesystem;

inline void CleanupRequest(HTTPRequestCompleted_t *pCallback, uint8 *pData)
{
    if (pData)
    {
        delete[] pData;
    }
    pData = nullptr;
    SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
}
#if 0

void MomentumUtil::DownloadMap(const char *szMapname)
{
    if (!SteamHTTP())
    {
        Warning("Failed to download map, cannot access HTTP!\n");
        return;
    }
    // MOM_TODO:
    // This should only be called if the user has the outdated map version or
    // doesn't have the map at all

    // The two different URLs:
    // cdn.momentum-mod.org/maps/MAPNAME/MAPNAME.bsp
    // and
    // cdn.momentum-mod.org/maps/MAPNAME/MAPNAME.zon
    // We're going to need to build requests for and download both of these files

    // Uncomment the following when we build the URLS (MOM_TODO)
    // CreateAndSendHTTPReq(mapfileURL, &cbDownloadCallback, &MomentumUtil::DownloadCallback);
    // CreateAndSendHTTPReq(zonFileURL, &cbDownloadCallback, &MomentumUtil::DownloadCallback);
}
bool MomentumUtil::CreateAndSendHTTPReqWithPost(const char *szURL,
    CCallResult<MomentumUtil, HTTPRequestCompleted_t> *callback,
    CCallResult<MomentumUtil, HTTPRequestCompleted_t>::func_t func,
    KeyValues *params)
{
    bool bSuccess = false;
    if (steamapicontext && SteamHTTP())
    {
        HTTPRequestHandle handle = SteamHTTP()->CreateHTTPRequest(k_EHTTPMethodPOST, szURL);
        FOR_EACH_VALUE(params, p_value)
        {
            SteamHTTP()->SetHTTPRequestGetOrPostParameter(handle, p_value->GetName(),
                p_value->GetString());
        }

        SteamAPICall_t apiHandle;

        if (SteamHTTP()->SendHTTPRequest(handle, &apiHandle))
        {
            Warning("Report sent.\n");
            callback->Set(apiHandle, this, func);
            bSuccess = true;
        }
        else
        {
            Warning("Failed to send HTTP Request to report bug online!\n");
            SteamHTTP()->ReleaseHTTPRequest(handle); // GC
        }
    }
    else
    {
        Warning("Steamapicontext is not online!\n");
    }
    return bSuccess;
}

#endif

#ifdef CLIENT_DLL
void MomentumUtil::UpdatePaintDecalScale(float fNewScale)
{
    IMaterial *material = materials->FindMaterial("decals/paint_decal", TEXTURE_GROUP_DECAL);
    if (material != nullptr)
    {
        static unsigned int nScaleCache = 0;
        IMaterialVar *pVarScale = material->FindVarFast("$decalscale", &nScaleCache);

        if (pVarScale != nullptr)
        {
            float flNewValue = 0.35f * fNewScale;

            pVarScale->SetFloatValueFast(flNewValue);
            pVarScale->SetIntValueFast((int) flNewValue);
        }
    }
}
#endif

void MomentumUtil::FormatTime(float m_flSecondsTime, char *pOut, const int precision, const bool fileName, const bool negativeTime) const
{
    // We want the absolute value to format! Negatives (if any) should be added post-format!
    m_flSecondsTime = fabs(m_flSecondsTime);
    char separator = fileName ? '-' : ':'; // MOM_TODO: Think of a better char?
    const char *negative = negativeTime ? "-" : "";
    int hours = static_cast<int>(m_flSecondsTime / (60.0f * 60.0f));
    int minutes = static_cast<int>(fmod(m_flSecondsTime / 60.0f, 60.0f));
    int seconds = static_cast<int>(fmod(m_flSecondsTime, 60.0f));
    int millis = static_cast<int>(fmod(m_flSecondsTime, 1.0f) * 1000.0f);
    int hundredths = millis / 10;
    int tenths = millis / 100;

    switch (precision)
    {
    case 0:
        if (hours > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%s%d%c%02d%c%02d", negative, hours, separator, minutes, separator, seconds);
        else if (minutes > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%s%d%c%02d", negative, minutes, separator, seconds);
        else
            Q_snprintf(pOut, BUFSIZETIME, "%s%d", negative, seconds);
        break;
    case 1:
        if (hours > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%s%d%c%02d%c%02d.%d", negative, hours, separator, minutes, separator,
                       seconds, tenths);
        else if (minutes > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%s%d%c%02d.%d", negative, minutes, separator, seconds, tenths);
        else
            Q_snprintf(pOut, BUFSIZETIME, "%s%d.%d", negative, seconds, tenths);
        break;
    case 2:
        if (hours > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%s%d%c%02d%c%02d.%02d", negative, hours, separator, minutes, separator,
                       seconds, hundredths);
        else if (minutes > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%s%d%c%02d.%02d", negative, minutes, separator, seconds, hundredths);
        else
            Q_snprintf(pOut, BUFSIZETIME, "%s%d.%02d", negative, seconds, hundredths);
        break;
    case 3:
        if (hours > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%s%d%c%02d%c%02d.%03d", negative, hours, separator, minutes, separator,
                       seconds, millis);
        else if (minutes > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%s%d%c%02d.%03d", negative, minutes, separator, seconds, millis);
        else
            Q_snprintf(pOut, BUFSIZETIME, "%s%d.%03d", negative, seconds, millis);
        break;
    case 4:
        if (hours > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%s%d%c%02d%c%02d.%04d", negative, hours, separator, minutes, separator,
                       seconds, millis);
        else if (minutes > 0)
            Q_snprintf(pOut, BUFSIZETIME, "%s%d%c%02d.%04d", negative, minutes, separator, seconds, millis);
        else
            Q_snprintf(pOut, BUFSIZETIME, "%s%d.%04d", negative, seconds, millis);
        break;
    }
}

Color MomentumUtil::GetColorFromVariation(const float variation, float deadZone, const Color &normalcolor, const Color &increasecolor,
                                          const Color &decreasecolor) const
{
    // variation is current velocity minus previous velocity.
    Color pFinalColor = normalcolor;
    deadZone = abs(deadZone);

    if (variation < -deadZone) // our velocity decreased
        pFinalColor = decreasecolor;
    else if (variation > deadZone) // our velocity increased
        pFinalColor = increasecolor;

    return pFinalColor;
}

bool MomentumUtil::GetColorFromHex(const char *hexColor, Color &into)
{
    uint32 hex = strtoul(hexColor, nullptr, 16);
    int length = Q_strlen(hexColor);
    if (length < 8)
    {
        uint8 r = (hex & 0xFF0000) >> 16;
        uint8 g = (hex & 0x00FF00) >> 8;
        uint8 b = (hex & 0x0000FF);
        into.SetColor(r, g, b, 255);
        return true;
    }
    if (length == 8)
    {
        return GetColorFromHex(hex, into);
    }
    Warning("Error: Color format incorrect! Use hex code in format \"RRGGBB\" or \"RRGGBBAA\"\n");
    return false;
}

bool MomentumUtil::GetColorFromHex(uint32 hex, Color &into)
{
    uint8 r = (hex & 0xFF000000) >> 24;
    uint8 g = (hex & 0x00FF0000) >> 16;
    uint8 b = (hex & 0x0000FF00) >> 8;
    uint8 a = (hex & 0x000000FF);
    into.SetColor(r, g, b, a);
    return true;
}
uint32 MomentumUtil::GetHexFromColor(const char *hexColor)
{
    return strtoul(hexColor, nullptr, 16);
}
uint32 MomentumUtil::GetHexFromColor(const Color &color)
{
    uint32 redByte = ((color.r() & 0xff) << 24);
    uint32 greenByte = ((color.g() & 0xff) << 16);
    uint32 blueByte = ((color.b() & 0xff) << 8);
    uint32 aByte = (color.a() & 0xff);
    return redByte + greenByte + blueByte + aByte;
}

void MomentumUtil::GetHexStringFromColor(const Color& color, char* pBuffer, int maxLen)
{
    const uint32 colorHex = GetHexFromColor(color);
    Q_snprintf(pBuffer, maxLen, "%08x", colorHex);
}

inline bool CheckReplayB(CMomReplayBase *pFastest, CMomReplayBase *pCheck, float tickrate, uint32 flags)
{
    if (pCheck)
    {
        if (pCheck->GetRunFlags() == flags && CloseEnough(tickrate, pCheck->GetTickInterval(), FLT_EPSILON))
        {
            if (pFastest)
            {
                return pCheck->GetRunTime() < pFastest->GetRunTime();
            }
            
            return true;
        }
    }

    return false;
}

//!!! NOTE: The value returned here MUST BE DELETED, otherwise you get a memory leak!
CMomReplayBase *MomentumUtil::GetBestTime(const char *szMapName, float tickrate, uint32 flags) const
{
    if (szMapName)
    {
        char path[MAX_PATH];
        Q_snprintf(path, MAX_PATH, "%s/%s-*%s", RECORDING_PATH, szMapName, EXT_RECORDING_FILE);
        V_FixSlashes(path);

        CMomReplayBase *pFastest = nullptr;

        FileFindHandle_t found;
        const char *pFoundFile = filesystem->FindFirstEx(path, "MOD", &found);
        while (pFoundFile)
        {
            // NOTE: THIS NEEDS TO BE MANUALLY CLEANED UP!
            char pReplayPath[MAX_PATH];
            V_ComposeFileName(RECORDING_PATH, pFoundFile, pReplayPath, MAX_PATH);

            CMomReplayBase *pBase = g_ReplayFactory.LoadReplayFile(pReplayPath, false);
            assert(pBase != nullptr);
                
            if (CheckReplayB(pFastest, pBase, tickrate, flags))
            {
                pFastest = pBase;
            }
            else // Not faster, get rid of it
            {
                delete pBase;
            }

            pFoundFile = filesystem->FindNext(found);
        }

        filesystem->FindClose(found);
        return pFastest;
    }
    return nullptr;
}

bool MomentumUtil::GetRunComparison(const char *szMapName, const float tickRate, const int flags, RunCompare_t *into) const
{
    if (into && szMapName)
    {
        CMomReplayBase *bestRun = GetBestTime(szMapName, tickRate, flags);
        if (bestRun)
        {
            // MOM_TODO: this may not be a PB, for now it is, but we'll load times from online.
            // I'm thinking the name could be like "(user): (Time)"
            FillRunComparison("Personal Best", bestRun->GetRunStats(), into);
            delete bestRun;
            DevLog("Loaded run comparisons for %s !\n", into->runName);
            return true;
        }
    }
    return false;
}

void MomentumUtil::FillRunComparison(const char *compareName, CMomRunStats *pRun, RunCompare_t *into) const
{
    Q_strcpy(into->runName, compareName);
    pRun->FullyCopyStats(&into->runStatsData);
}

#define SAVE_3D_TO_KV(kvInto, pName, toSave)                                                                           \
    if (!kvInto || !pName)                                                                                             \
        return;                                                                                                        \
    char value[512];                                                                                                   \
    Q_snprintf(value, 512, "%f %f %f", toSave.x, toSave.y, toSave.z);                                                  \
    kvInto->SetString(pName, value);

#define LOAD_3D_FROM_KV(kvFrom, pName, into)                                                                           \
    if (!kvFrom || !pName)                                                                                             \
        return;                                                                                                        \
    sscanf(kvFrom->GetString(pName), "%f %f %f", &into.x, &into.y, &into.z);

void MomentumUtil::KVSaveVector(KeyValues *kvInto, const char *pName, const Vector &toSave)
{
    SAVE_3D_TO_KV(kvInto, pName, toSave);
}

void MomentumUtil::KVLoadVector(KeyValues *kvFrom, const char *pName, Vector &vecInto)
{
    LOAD_3D_FROM_KV(kvFrom, pName, vecInto);
}

void MomentumUtil::KVSaveQAngles(KeyValues *kvInto, const char *pName, const QAngle &toSave)
{
    SAVE_3D_TO_KV(kvInto, pName, toSave);
}

void MomentumUtil::KVLoadQAngles(KeyValues *kvFrom, const char *pName, QAngle &angInto)
{
    LOAD_3D_FROM_KV(kvFrom, pName, angInto);
}

inline void FindHullIntersection(const Vector &vecSrc, trace_t &tr, const Vector &mins, const Vector &maxs, CBaseEntity *pEntity)
{
    int			i, j, k;
    float		distance;
    Vector minmaxs[2] = { mins, maxs };
    trace_t tmpTrace;
    Vector		vecHullEnd = tr.endpos;
    Vector		vecEnd;

    distance = 1e6f;

    vecHullEnd = vecSrc + ((vecHullEnd - vecSrc) * 2);
    UTIL_TraceLine(vecSrc, vecHullEnd, MASK_SOLID, pEntity, COLLISION_GROUP_NONE, &tmpTrace);
    if (tmpTrace.fraction < 1.0)
    {
        tr = tmpTrace;
        return;
    }

    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 2; j++)
        {
            for (k = 0; k < 2; k++)
            {
                vecEnd.x = vecHullEnd.x + minmaxs[i][0];
                vecEnd.y = vecHullEnd.y + minmaxs[j][1];
                vecEnd.z = vecHullEnd.z + minmaxs[k][2];

                UTIL_TraceLine(vecSrc, vecEnd, MASK_SOLID, pEntity, COLLISION_GROUP_NONE, &tmpTrace);
                if (tmpTrace.fraction < 1.0)
                {
                    float thisDistance = (tmpTrace.endpos - vecSrc).Length();
                    if (thisDistance < distance)
                    {
                        tr = tmpTrace;
                        distance = thisDistance;
                    }
                }
            }
        }
    }
}

void MomentumUtil::KnifeTrace(const Vector& vecShootPos, const QAngle& lookAng, bool bStab, CBaseEntity *pAttacker,
    CBaseEntity *pSoundSource, trace_t* trOutput, Vector* vForwardOut)
{
    float fRange = bStab ? 32.0f : 48.0f; // knife range

    AngleVectors(lookAng, vForwardOut);
    Vector vecSrc = vecShootPos;
    Vector vecEnd = vecSrc + *vForwardOut * fRange;

    UTIL_TraceLine(vecSrc, vecEnd, MASK_SOLID, pAttacker, COLLISION_GROUP_NONE, trOutput);

    //check for hitting glass
#ifndef CLIENT_DLL
    CTakeDamageInfo glassDamage(pAttacker, pAttacker, 42.0f, DMG_BULLET | DMG_NEVERGIB);
    pSoundSource->TraceAttackToTriggers(glassDamage, trOutput->startpos, trOutput->endpos, *vForwardOut);
#endif

    if (trOutput->fraction >= 1.0)
    {
        Vector head_hull_mins(-16, -16, -18);
        Vector head_hull_maxs(16, 16, 18);
        UTIL_TraceHull(vecSrc, vecEnd, head_hull_mins, head_hull_maxs, MASK_SOLID, pAttacker, COLLISION_GROUP_NONE, trOutput);
        if (trOutput->fraction < 1.0)
        {
            // Calculate the point of intersection of the line (or hull) and the object we hit
            // This is and approximation of the "best" intersection
            CBaseEntity *pHit = trOutput->m_pEnt;
            if (!pHit || pHit->IsBSPModel())
                FindHullIntersection(vecSrc, *trOutput, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, pAttacker);
            //vecEnd = trOutput->endpos;	// This is the point on the actual surface (the hull could have hit space)
        }
    }

    bool bDidHit = trOutput->fraction < 1.0f;


    if (!bDidHit)
    {
        // play wiff or swish sound
        CPASAttenuationFilter filter(pSoundSource);
        filter.UsePredictionRules();
        CBaseEntity::EmitSound(filter, pSoundSource->entindex(), "Weapon_Knife.Slash");
    }
#ifndef CLIENT_DLL
    else
    {
        // play thwack, smack, or dong sound

        CBaseEntity *pEntity = trOutput->m_pEnt;

        ClearMultiDamage();

        float flDamage = 42.0f;

        if ( bStab )
        {
            flDamage = 65.0f;

            if ( pEntity && pEntity->IsPlayer() )
            {
                Vector vTragetForward;

                AngleVectors( pEntity->GetAbsAngles(), &vTragetForward );

                Vector2D vecLOS = (pEntity->GetAbsOrigin() - pAttacker->GetAbsOrigin()).AsVector2D();
                Vector2DNormalize( vecLOS );

                float flDot = vecLOS.Dot( vTragetForward.AsVector2D() );

                //Triple the damage if we are stabbing them in the back.
                if ( flDot > 0.80f )
                    flDamage *= 3.0f;
            }
        }
        else
        {
            /*if ( bFirstSwing )
            {
                // first swing does full damage
                flDamage = 20;
            }
            else
            {
                // subsequent swings do less	
                flDamage = 15;
            }*/
            flDamage = 20.0f;
        }

        CTakeDamageInfo info( pAttacker, pAttacker, flDamage, DMG_BULLET | DMG_NEVERGIB );

        CalculateMeleeDamageForce( &info, *vForwardOut, trOutput->endpos, 1.0f/flDamage );
        pEntity->DispatchTraceAttack( info, *vForwardOut, trOutput ); 
        ApplyMultiDamage();
    }
#endif
}

void MomentumUtil::KnifeSmack(const trace_t& trIn, CBaseEntity *pSoundSource, const QAngle& lookAng, const bool bStab)
{
    if (!trIn.m_pEnt || (trIn.surface.flags & SURF_SKY))
        return;

    if (trIn.fraction == 1.0)
        return;

    if (trIn.m_pEnt)
    {
        CPASAttenuationFilter filter(pSoundSource);
        filter.UsePredictionRules();

        if (trIn.m_pEnt->IsPlayer())
        {
            pSoundSource->EmitSound(filter, pSoundSource->entindex(), bStab ? "Weapon_Knife.Stab" : "Weapon_Knife.Hit");
        }
        else
        {
            pSoundSource->EmitSound(filter, pSoundSource->entindex(), "Weapon_Knife.HitWall");
        }
    }

    CEffectData data;
    data.m_vOrigin = trIn.endpos;
    data.m_vStart = trIn.startpos;
    data.m_nSurfaceProp = trIn.surface.surfaceProps;
    data.m_nDamageType = DMG_SLASH;
    data.m_nHitBox = trIn.hitbox;
#ifdef CLIENT_DLL
    data.m_hEntity = trIn.m_pEnt->GetRefEHandle();
#else
    data.m_nEntIndex = trIn.m_pEnt->entindex();
#endif

    CPASFilter filter(data.m_vOrigin);
    data.m_vAngles = lookAng;
    data.m_fFlags = 0x1;	//IMPACT_NODECAL;

    te->DispatchEffect(filter, 0.0, data.m_vOrigin, "KnifeSlash", data);
}

static MomentumUtil s_momentum_util;
MomentumUtil *g_pMomentumUtil = &s_momentum_util;
