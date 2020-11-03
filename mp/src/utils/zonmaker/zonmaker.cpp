
#include "chunkfile.h"
#include "cmdlib.h"
#include "tier1/KeyValues.h"
#include "tier1/fmtstr.h"
#include "tier1/strtools.h"
#include "tier1/utlbuffer.h"
#include "tier1/utlstring.h"
#include "tier1/utlcommon.h"

#include "../../game/shared/momentum/mom_shareddefs.h"

#include "tier0/memdbgon.h"

using CKeyValue = CUtlKeyValuePair<CUtlString, CUtlString>;
struct CChunk
{
    CChunk(const char *chunkName) : ChunkName(chunkName){}
    ~CChunk()
    {
        Chunks.PurgeAndDeleteElements();
        Keys.PurgeAndDeleteElements();
    }

    const CUtlString &FindKey(char const *pKeyName) const;
    const CChunk *FindChunk(char const *pChunkName) const;

    const CUtlString ChunkName;
    CUtlVector<CKeyValue *> Keys;
    CUtlVector<CChunk *> Chunks;
};

const CUtlString &CChunk::FindKey(char const *pKeyName) const
{
    for (auto pKey : Keys)
    {
        if (pKey->m_key.IsEqual_CaseInsensitive(pKeyName))
            return pKey->GetValue();
    }

    return CUtlString::GetEmptyString();
}

const CChunk *CChunk::FindChunk(char const *pChunkName) const
{
    for (auto pChunk : Chunks)
    {
        if (pChunk->ChunkName.IsEqual_CaseInsensitive(pChunkName))
            return pChunk;
    }

    return nullptr;
}

CChunk *g_pCurChunk = nullptr;
static ChunkFileResult_t MyKeyHandler(const char *szKey, const char *szValue, void *pData)
{
    g_pCurChunk->Keys.AddToTail(new CKeyValue(szKey, szValue));
    return ChunkFile_Ok;
}

uint32 g_DotCounter = 0;
static CChunk *ParseChunk(CChunkFile *pFile, char const *pChunkName, bool bOnlyOne)
{
    // Add the new chunk.
    CChunk *pChunk = new CChunk(pChunkName);

    // Parse it out..
    CChunk *pOldChunk = g_pCurChunk;
    g_pCurChunk = pChunk;

    while (true)
    {
        if (++g_DotCounter % 512 == 0)
            Msg(".");

        if (pFile->ReadChunk(MyKeyHandler) != ChunkFile_Ok)
            break;

        if (bOnlyOne)
            break;
    }

    g_pCurChunk = pOldChunk;
    return pChunk;
}

static ChunkFileResult_t MyDefaultHandler(CChunkFile *pFile, void *pData, char const *pChunkName)
{
    g_pCurChunk->Chunks.AddToTail(ParseChunk(pFile, pChunkName, true));
    return ChunkFile_Ok;
}

static CChunk *ReadChunkFile(char const *pInFilename)
{
    CChunkFile chunkFile;
    if (chunkFile.Open(pInFilename, ChunkFile_Read) != ChunkFile_Ok)
    {
        Msg("Error opening %s for reading.\n", pInFilename);
        return nullptr;
    }

    Msg("Reading..");
    chunkFile.SetDefaultChunkHandler(MyDefaultHandler, nullptr);

    CChunk *pRet = ParseChunk(&chunkFile, "ROOT", false);
    Msg("\n");

    return pRet;
}

static Vector UTIL_StringToFloatArray(const char *pString)
{
    const char *pfront;
    const char *pstr = pfront = pString;

    Vector vector;
    int j;
    for (j = 0; j < 3; j++) // lifted from pr_edict.c
    {
        vector[j] = atof(pfront);

        // skip any leading whitespace
        while (*pstr && *pstr <= ' ')
            pstr++;

        // skip to next whitespace
        while (*pstr && *pstr > ' ')
            pstr++;

        if (!*pstr)
            break;

        pstr++;
        pfront = pstr;
    }
    for (j++; j < 3; j++)
        vector[j] = 0;
    return vector;
}

// matches order of MomZoneType_t
constexpr const char *validTriggers[] = {
    "trigger_momentum_timer_stop",
    "trigger_momentum_timer_start",
    "trigger_momentum_timer_stage",
    "trigger_momentum_timer_checkpoint",
    "trigger_momentum_trick"
};

static KeyValues *ParseZones(CChunk *pRootChunk)
{
    auto kv = new KeyValues("tracks");
    for (CChunk *pChunk : pRootChunk->Chunks)
    {
        if (!pChunk->ChunkName.IsEqual_CaseInsensitive("entity"))
            continue;
        const auto &className = pChunk->FindKey("classname");
        if (className.IsEmpty())
            continue;
        uint type = 0;
        for (; type < ARRAYSIZE(validTriggers); ++type)
        {
            if (className.IsEqual_CaseInsensitive(validTriggers[type]))
                break;
        }
        if (type == ARRAYSIZE(validTriggers))
            continue;

        const auto solid = pChunk->FindChunk("solid");
        if (!solid)
            continue;

        float height = -1.f;
        CUtlVector<Vector> points;
        for (const auto side : solid->Chunks)
        {
            if (!side->ChunkName.IsEqual_CaseInsensitive("side"))
                continue;

            const auto facePoints = side->FindChunk("point_data");
            if (!facePoints)
                continue;

            CUtlVector<Vector> planePoints;
            for (const auto p : facePoints->Keys)
            {
                if (!p->m_key.IsEqual_CaseInsensitive("point"))
                    continue;

                int s;
                Vector point;
                sscanf_s(p->GetValue(), "%d %f %f %f", &s, &point.x, &point.y, &point.z);
                if (planePoints.Count() < s + 1)
                    planePoints.SetCountNonDestructively(s + 1);

                planePoints[s] = point;
            }

            bool onSameZ = true;
            const float z = planePoints[0].z;
            for (int i = 1; i < planePoints.Count(); ++i)
            {
                if (!CloseEnough(z, planePoints[i].z))
                {
                    onSameZ = false;
                    break;
                }
            }
            if (!onSameZ)
                continue;
            if (points.IsEmpty())
                points = planePoints;
            else
            {
                if (points[0].z > planePoints[0].z)
                {
                    height = points[0].z - planePoints[0].z;
                    points = planePoints; // save the lower plane
                }
                else
                    height = planePoints[0].z - points[0].z;
                break; // were done, have two parallel planes
            }
        }

        if (points.IsEmpty() || height <= 0.f)
        {
            Warning("Zone '%s' without points!\n", className.Get());
            continue;
        }

        const auto &trackNumber = pChunk->FindKey("track_number");
        const auto &zoneNumber = pChunk->FindKey("zone_number");

        auto trackKV = kv->FindKey(trackNumber, true);
        const auto zoneN = type == ZONE_TYPE_START ? "1" : type == ZONE_TYPE_STOP ? "0" : zoneNumber.Get();
        auto zoneKV = trackKV->FindKey(zoneN, true);
        if (zoneKV->IsEmpty())
            zoneKV->SetString("zoneNum", zoneN);
        auto triggerKV = zoneKV->FindKey("triggers", true)->CreateNewKey();

        triggerKV->SetInt("type", type);
        triggerKV->SetString("zoneNum", zoneN);
        if (type == ZONE_TYPE_START)
        {
            auto propsKV = triggerKV->FindKey("zoneProps", true);

            const auto spawnFlags = V_atoi(pChunk->FindKey("spawnflags"));
            if (const auto &key = pChunk->FindKey("speed_limit"))
                propsKV->SetString("speed_limit", key);
            else
                propsKV->SetFloat("speed_limit", 350.f);
            propsKV->SetBool("limiting_speed", (spawnFlags & 8192) != 0); // SF_LIMIT_LEAVE_SPEED

            if (const auto &key = pChunk->FindKey("start_on_jump"))
                propsKV->SetString("start_on_jump", key);
            else
                propsKV->SetBool("start_on_jump", true);

            if ( const auto &key = pChunk->FindKey("speed_limit_type"))
                propsKV->SetString("speed_limit_type", key);
            else
                propsKV->SetInt("speed_limit_type", 0);
            if (spawnFlags & 16384) // SF_USE_LOOKANGLES
            {
                const auto angles = UTIL_StringToFloatArray(pChunk->FindKey("look_angles"));
                propsKV->SetFloat("yaw", angles[YAW]);
            }
        }
        else if (type == ZONE_TYPE_TRICK)
        {
            const auto &name = pChunk->FindKey("name");
            triggerKV->SetString("name", name ? name.Get() : "NOT NAMED!!!!! ERROR!");
        }

        triggerKV->SetFloat("pointsHeight", height);
        triggerKV->SetFloat("pointsZPos", points[0].z);
        auto pointsKV = triggerKV->FindKey("points", true);
        CFmtStr f;
        for (int i = 0; i < points.Count(); ++i)
        {
            const auto &p = points[i];
            pointsKV->SetString(f.sprintf("p%d", i), CFmtStr("%.3f %.3f", p.x, p.y));
        }
    }

    return kv;
}

int main(int argc, char *argv[])
{
    InstallAllocationFunctions();
    InstallSpewFunction();

    if (argc < 2)
    {
        Warning("zonmaker <input file>\n");
        return 1;
    }

    char const *pInFilename = argv[1];
    char name[MAX_PATH * 2];
    V_strcpy_safe(name, pInFilename);
    V_SetExtension(name, EXT_ZONE_FILE, MAX_PATH * 2);

    CChunk *pRoot = ReadChunkFile(pInFilename);
    if (!pRoot)
        return 2;

    KeyValuesAD zones{ParseZones(pRoot)};

    delete pRoot;

    if (!zones->IsEmpty())
    {
        CUtlBuffer buf{0, 0, CUtlBuffer::TEXT_BUFFER};
        zones->RecursiveSaveToFile(buf, 0, 2);

        FILE *file;
        if (fopen_s(&file, name, "w") == 0)
        {
            fwrite(buf.String(), sizeof(char), buf.TellPut(), file);
            fclose(file);
        }
        Msg("Done!\n");
    }
    else
        Warning("Couldn't parse any zone information from VMF!\n");

    return 0;
}