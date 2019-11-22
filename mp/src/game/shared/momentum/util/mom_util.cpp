#include "cbase.h"

#include <ctime>
#include "filesystem.h"
#include "utlbuffer.h"
#include "mom_util.h"
#include "momentum/mom_shareddefs.h"
#include "run/mom_replay_factory.h"
#include "run/mom_replay_base.h"
#include "run/run_compare.h"
#include "run/run_stats.h"
#include "run/mom_run_entity.h"
#include "effect_dispatch_data.h"
#ifdef CLIENT_DLL
#include "materialsystem/imaterialvar.h"
#else
#include "momentum/mom_player.h"
#endif

#include "steam/steam_api.h"
#include "fmtstr.h"

#include "tier0/valve_minmax_off.h"
// These are wrapped by minmax_off/on due to Valve making a macro for min and max...
#include "cryptopp/sha.h"
#include <cryptopp/files.h>
#include <cryptopp/hex.h>
// Now we can unwrap
#include "tier0/valve_minmax_on.h"

#include "tier0/memdbgon.h"

extern IFileSystem *filesystem;

#ifdef CLIENT_DLL
void MomUtil::UpdatePaintDecalScale(float fNewScale)
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

void MomUtil::DispatchConCommand(const char *pszCommand)
{
    CCommand args;
    args.Tokenize(pszCommand);

    ConCommand *pCommand = g_pCVar->FindCommand(args[0]);
    if (pCommand) // can we directly call this command?
    {
        pCommand->Dispatch(args);
    }
    else // fallback to old code
    {
        engine->ClientCmd_Unrestricted(pszCommand);
    }
}
#endif

void MomUtil::MountGameFiles()
{
    if (SteamApps())
    {
        char installPath[MAX_PATH];
        uint32 folderLen;

        // CS:S
        folderLen = SteamApps()->GetAppInstallDir(240, installPath, MAX_PATH);
        if (folderLen)
        {
            filesystem->AddSearchPath(CFmtStr("%s/cstrike", installPath), "GAME");
            filesystem->AddSearchPath(CFmtStr("%s/cstrike/cstrike_pak.vpk", installPath), "GAME");
            filesystem->AddSearchPath(CFmtStr("%s/cstrike/download", installPath), "GAME");
            filesystem->AddSearchPath(CFmtStr("%s/cstrike/download", installPath), "download");
        }

        // TF2
        folderLen = SteamApps()->GetAppInstallDir(440, installPath, MAX_PATH);
        if (folderLen)
        {
            filesystem->AddSearchPath(CFmtStr("%s/tf", installPath), "GAME");
            filesystem->AddSearchPath(CFmtStr("%s/tf/tf2_misc.vpk", installPath), "GAME");
            filesystem->AddSearchPath(CFmtStr("%s/tf/tf2_sound_misc.vpk", installPath), "GAME");
            filesystem->AddSearchPath(CFmtStr("%s/tf/tf2_sound_vo_english.vpk", installPath), "GAME");
            filesystem->AddSearchPath(CFmtStr("%s/tf/tf2_textures.vpk", installPath), "GAME");
            filesystem->AddSearchPath(CFmtStr("%s/tf/download", installPath), "GAME");
            filesystem->AddSearchPath(CFmtStr("%s/tf/download", installPath), "download");
        }

        if (developer.GetInt())
            filesystem->PrintSearchPaths();
    }
}

void MomUtil::FormatTime(float m_flSecondsTime, char *pOut, const int precision, const bool fileName, const bool negativeTime)
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
    default:
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

bool MomUtil::GetTimeAgoString(time_t *input, char* pOut, size_t outLen)
{
    if (!input)
        return false;

    const char *pUnitString = nullptr;
    int count = 0;
    time_t now;
    time(&now);
    const auto diff = difftime(now, *input); // Diff in seconds

    const int years = static_cast<int>(diff / 31557600.0f);
    const int months = static_cast<int>(diff / 2629800.0f); // Average number of seconds per month
    const int days = static_cast<int>(diff / 86400.0f);
    const int hours = static_cast<int>(diff / 3600.0f);
    const int minutes = static_cast<int>(diff / 60.0f);
    if (years)
    {
        pUnitString = "year";
        count = years;
    }
    else if (months)
    {
        pUnitString = "month";
        count = months;
    }
    else if (days) 
    {
        pUnitString = "day";
        count = days;
    }
    else if (hours)
    {
        pUnitString = "hour";
        count = hours;
    }
    else if (minutes)
    {
        pUnitString = "minute";
        count = minutes;
    }
    else if (diff)
    {
        pUnitString = "second";
        count = diff;
    }

    if (!pUnitString || count <= 0)
        Q_strncpy(pOut, "just now", outLen);
    else
        Q_snprintf(pOut, outLen, "%i %s%s ago", count, pUnitString, (count > 1 ? "s" : ""));

    return true;
}

bool MomUtil::GetTimeAgoString(const char* pISODate, char* pOut, size_t outLen)
{
    time_t temp;
    if (ISODateToTimeT(pISODate, &temp))
    {
        return GetTimeAgoString(&temp, pOut, outLen);
    }
    return false;
}

bool MomUtil::ISODateToTimeT(const char* pISODate, time_t* out)
{
    if (!pISODate || pISODate[0] == '\0')
        return false;
    int year, month, day, hour, min, sec, millis;
    sscanf(pISODate, "%d-%d-%dT%d:%d:%d.%dZ", &year, &month, &day, &hour, &min, &sec, &millis);
    tm tim;
    tim.tm_year = year - 1900;
    tim.tm_mday = day;
    tim.tm_mon = month - 1;
    tim.tm_hour = hour;
    tim.tm_min = min;
    tim.tm_sec = sec;
    tim.tm_isdst = 0;

    *out = mktime(&tim) -
#ifdef WIN32 
        _timezone;
#else
        timezone;
#endif
    return true;
}

Color MomUtil::GetColorFromVariation(const float variation, float deadZone, const Color &normalcolor, const Color &increasecolor,
                                          const Color &decreasecolor)
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

Color MomUtil::ColorLerp(float prog, const Color& A, const Color& B)
{
    // To linear color
    float A0 = (float) A[0] / 255.0;
    float A1 = (float) A[1] / 255.0;
    float A2 = (float) A[2] / 255.0;
    float A3 = (float) A[3] / 255.0;

    float B0 = (float) B[0] / 255.0;
    float B1 = (float) B[1] / 255.0;
    float B2 = (float) B[2] / 255.0;
    float B3 = (float) B[3] / 255.0;

    // Lerping colors
    Color ret;
    ret[0] = static_cast<unsigned char>(Lerp(prog, A0, B0) * 255.0f);
    ret[1] = static_cast<unsigned char>(Lerp(prog, A1, B1) * 255.0f);
    ret[2] = static_cast<unsigned char>(Lerp(prog, A2, B2) * 255.0f);
    ret[3] = static_cast<unsigned char>(Lerp(prog, A3, B3) * 255.0f);
    return ret;
}

bool MomUtil::GetColorFromHex(const char *hexColor, Color &into)
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

bool MomUtil::GetColorFromHex(uint32 hex, Color &into)
{
    uint8 r = (hex & 0xFF000000) >> 24;
    uint8 g = (hex & 0x00FF0000) >> 16;
    uint8 b = (hex & 0x0000FF00) >> 8;
    uint8 a = (hex & 0x000000FF);
    into.SetColor(r, g, b, a);
    return true;
}
uint32 MomUtil::GetHexFromColor(const char *hexColor)
{
    return strtoul(hexColor, nullptr, 16);
}
uint32 MomUtil::GetHexFromColor(const Color &color)
{
    uint32 redByte = ((color.r() & 0xff) << 24);
    uint32 greenByte = ((color.g() & 0xff) << 16);
    uint32 blueByte = ((color.b() & 0xff) << 8);
    uint32 aByte = (color.a() & 0xff);
    return redByte + greenByte + blueByte + aByte;
}

void MomUtil::GetHexStringFromColor(const Color& color, char* pBuffer, int maxLen)
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
CMomReplayBase *MomUtil::GetBestTime(const char *szMapName, float tickrate, uint32 flags)
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

bool MomUtil::GetRunComparison(const char *szMapName, const float tickRate, const int flags, RunCompare_t *into)
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

void MomUtil::FillRunComparison(const char *compareName, CMomRunStats *pRun, RunCompare_t *into)
{
    Q_strcpy(into->runName, compareName);
    into->runStats.FullyCopyFrom(*pRun);
}

bool MomUtil::IsInBounds(const Vector2D &source, const Vector2D &bottomLeft, const Vector2D &topRight)
{
    return (source.x > bottomLeft.x && source.x < topRight.x) && (source.y > bottomLeft.y && source.y < topRight.y);
}

bool MomUtil::IsInBounds(const int x, const int y, const int rectX, const int rectY, const int rectW, const int rectH)
{
    return IsInBounds(Vector2D(x, y), Vector2D(rectX, rectY), Vector2D(rectX + rectW, rectY + rectH));
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

void MomUtil::KVSaveVector(KeyValues *kvInto, const char *pName, const Vector &toSave)
{
    SAVE_3D_TO_KV(kvInto, pName, toSave);
}

void MomUtil::KVLoadVector(KeyValues *kvFrom, const char *pName, Vector &vecInto)
{
    LOAD_3D_FROM_KV(kvFrom, pName, vecInto);
}

void MomUtil::KVSaveQAngles(KeyValues *kvInto, const char *pName, const QAngle &toSave)
{
    SAVE_3D_TO_KV(kvInto, pName, toSave);
}

void MomUtil::KVLoadQAngles(KeyValues *kvFrom, const char *pName, QAngle &angInto)
{
    LOAD_3D_FROM_KV(kvFrom, pName, angInto);
}

inline void FindHullIntersection(const Vector &vecSrc, trace_t &tr, const Vector &mins, const Vector &maxs, CBaseEntity *pEntity)
{
    int     i, j, k;
    float   distance;
    Vector  minmaxs[2] = { mins, maxs };
    trace_t tmpTrace;
    Vector  vecHullEnd = tr.endpos;
    Vector  vecEnd;

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

void MomUtil::KnifeTrace(const Vector& vecShootPos, const QAngle& lookAng, bool bStab, CBaseEntity *pAttacker,
    CBaseEntity *pSoundSource, trace_t* trOutput, Vector* vForwardOut)
{
    float fRange = bStab ? 32.0f : 48.0f; // knife range

    AngleVectors(lookAng, vForwardOut);
    Vector vecSrc = vecShootPos;
    Vector vecEnd = vecSrc + *vForwardOut * fRange;

    UTIL_TraceLine(vecSrc, vecEnd, MASK_SOLID, pAttacker, COLLISION_GROUP_NONE, trOutput);

#ifndef CLIENT_DLL
    if (pAttacker->IsPlayer())
    {
        //check for hitting glass
        CTakeDamageInfo glassDamage(pAttacker, pAttacker, 42.0f, DMG_BULLET | DMG_NEVERGIB);
        pSoundSource->TraceAttackToTriggers(glassDamage, trOutput->startpos, trOutput->endpos, *vForwardOut);
    }
#endif

    if (trOutput->fraction >= 1.0f)
    {
        Vector head_hull_mins(-16, -16, -18);
        Vector head_hull_maxs(16, 16, 18);
        UTIL_TraceHull(vecSrc, vecEnd, head_hull_mins, head_hull_maxs, MASK_SOLID, pAttacker, COLLISION_GROUP_NONE, trOutput);
        if (trOutput->fraction < 1.0f)
        {
            // Calculate the point of intersection of the line (or hull) and the object we hit
            // This is and approximation of the "best" intersection
            CBaseEntity *pHit = trOutput->m_pEnt;
            if (!pHit || pHit->IsBSPModel())
                FindHullIntersection(vecSrc, *trOutput, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, pAttacker);
            //vecEnd = trOutput->endpos; // This is the point on the actual surface (the hull could have hit space)
        }
    }

    bool bDidHit = trOutput->fraction < 1.0f;

    if (!bDidHit)
    {
        // play wiff or swish sound
        CPASAttenuationFilter filter(trOutput->endpos, "Weapon_Knife.Slash");
        filter.UsePredictionRules();
        CBaseEntity::EmitSound(filter, pSoundSource->entindex(), "Weapon_Knife.Slash");
    }
#ifndef CLIENT_DLL
    else if (pAttacker->IsPlayer())
    {
        CMomentumPlayer *pPlayer = static_cast<CMomentumPlayer*>(pAttacker);
        if (pPlayer->m_bHasPracticeMode)
            return;

        CBaseEntity *pHitEntity = trOutput->m_pEnt;

        ClearMultiDamage();

        const float flDamage = bStab ? 65.0f : 20.0f;

        CTakeDamageInfo info( pAttacker, pAttacker, flDamage, DMG_BULLET | DMG_NEVERGIB );

        CalculateMeleeDamageForce( &info, *vForwardOut, trOutput->endpos, 1.0f/flDamage );
        pHitEntity->DispatchTraceAttack(info, *vForwardOut, trOutput); 
        ApplyMultiDamage();
    }
#endif
}

void MomUtil::KnifeSmack(const trace_t& trIn, CBaseEntity *pSoundSource, const QAngle& lookAng, const bool bStab)
{
    if (!trIn.m_pEnt || (trIn.surface.flags & SURF_SKY))
        return;

    if (trIn.fraction == 1.0f)
        return;

    if (trIn.m_pEnt)
    {
        CPASAttenuationFilter filter(trIn.endpos);
        filter.UsePredictionRules();

        const auto pRunEnt = dynamic_cast<CMomRunEntity*>(trIn.m_pEnt);
        if (pRunEnt)
        {
            CBaseEntity::EmitSound(filter, pSoundSource->entindex(), bStab ? "Weapon_Knife.Stab" : "Weapon_Knife.Hit");
        }
        else
        {
            CBaseEntity::EmitSound(filter, pSoundSource->entindex(), "Weapon_Knife.HitWall");
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
    data.m_fFlags = 0x1; //IMPACT_NODECAL;

    te->DispatchEffect(filter, 0.0, data.m_vOrigin, "KnifeSlash", data);
}

bool MomUtil::GetSHA1Hash(const CUtlBuffer& buf, char* pOut, size_t outLen)
{
    CryptoPP::SHA1 hash;
    byte digest[CryptoPP::SHA1::DIGESTSIZE];
    hash.CalculateDigest(digest, (const byte*) buf.Base(), buf.TellPut());
    std::string output;
    CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(output), false, 0, "");
    encoder.Put(digest, sizeof(digest));
    encoder.MessageEnd();
    Q_strncpy(pOut, output.c_str(), outLen);
    return true;
}

bool MomUtil::GetFileHash(char* pOut, size_t outLen, const char *pFileName, const char *pPathID /* = "GAME"*/)
{
    CUtlBuffer fileBuffer;
    if (g_pFullFileSystem->ReadFile(pFileName, pPathID, fileBuffer))
        return GetSHA1Hash(fileBuffer, pOut, outLen);

    return false;
}

bool MomUtil::FileExists(const char* pFileName, const char* pFileHash, const char* pPathID /* = "GAME"*/)
{
    if (!(pFileName && pFileHash))
        return false;

    char hashDigest[41];
    if (GetFileHash(hashDigest, sizeof(hashDigest), pFileName, pPathID))
    {
        return FStrEq(hashDigest, pFileHash);
    }

    return false;
}

// Gross hack needed because scheme()->GetImage still returns an image even if it's null (returns the null texture)
bool MomUtil::MapThumbnailExists(const char* pMapName)
{
    if (!pMapName) return false;
    char szPath[MAX_PATH];
    Q_snprintf(szPath, MAX_PATH, "materials/vgui/maps/%s.vmt", pMapName);
    return g_pFullFileSystem->FileExists(szPath, "GAME");
}