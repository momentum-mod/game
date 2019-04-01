#include "cbase.h"
#include "filesystem.h"
#include "mapzones.h"
#include "mom_timer.h"
#include "mom_triggers.h"
#include "mapzones_build.h"
#include "fmtstr.h"

#include "tier0/memdbgon.h"

static void SaveZonFile(const char *pMapName);
static void CC_Mom_GenerateZoneFile() { SaveZonFile(gpGlobals->mapname.ToCStr()); }
static ConCommand mom_generate_zone_file("mom_zone_generate", CC_Mom_GenerateZoneFile, "Generates a zone file.");

class CMapZone
{
  public:
    CMapZone(int type, int track, const KeyValues *values);
    ~CMapZone();

    void SpawnZone();

    int GetType() const { return m_iType; }

  private:
    int m_iType;
    int m_iTrack; // Track number
    // KeyValues containing all the values describing the zone
    KeyValues *m_pZoneValues;

    CBaseMomZoneTrigger *m_pTrigger;
};

CMapZone::CMapZone(const int type, const int track, const KeyValues *values)
{
    m_iType = type;
    m_iTrack = track;
    m_pZoneValues = values->MakeCopy();
    m_pTrigger = nullptr;
}

CMapZone::~CMapZone()
{
    m_pZoneValues->deleteThis();
}

void CMapZone::SpawnZone()
{
    char classname[64];
    ZoneTypeToClass(m_iType, classname, sizeof(classname));
    m_pTrigger = dynamic_cast<CBaseMomZoneTrigger *>(CreateEntityByName(classname));
    AssertMsg(m_pTrigger, "Unhandled zone type");

    if (m_pTrigger)
    {
        if (!m_pTrigger->LoadFromKeyValues(m_pZoneValues))
        {
            Warning("Failed to load zone of type '%s' (Invalid zone data)", classname);
            Assert(false);
            return;
        }

        m_pTrigger->SetTrackNumber(m_iTrack);

        CMomBaseZoneBuilder* pBaseBuilder = CreateZoneBuilderFromKeyValues(m_pZoneValues);

        pBaseBuilder->BuildZone();
        m_pTrigger->Spawn();
        pBaseBuilder->FinishZone(m_pTrigger);

        m_pTrigger->Activate();

        delete pBaseBuilder;
    }
}

static void SaveTrigger(CTriggerZone *pTrigger, KeyValues *pKvInto)
{
    bool bSuccess = false;
    const auto pKvZone = pKvInto->CreateNewKey();
    if (pTrigger->ToKeyValues(pKvZone))
    {
        auto pBuilder = CreateZoneBuilderFromExisting(pTrigger);

        bSuccess = pBuilder->Save(pKvZone);

        delete pBuilder;
    }

    if (!bSuccess)
    {
        Warning("Failed to save zone to file!\n");
        pKvInto->RemoveSubKey(pKvZone);
        pKvZone->deleteThis();
    }
}

static void SaveZonFile(const char *szMapName)
{
    KeyValuesAD zoneKV(szMapName);
    CTriggerZone *trackTriggers[MAX_TRACKS][MAX_ZONES];
    memset(trackTriggers, NULL, sizeof(trackTriggers[0][0]) * MAX_TRACKS * MAX_ZONES);
    CUtlVector<CTriggerZone*> versatileTriggers;

    CBaseEntity *pEnt = gEntList.FindEntityByClassname(nullptr, "trigger_momentum_timer_*");
    while (pEnt)
    {
        CTriggerZone *pTrigger = dynamic_cast<CTriggerZone*>(pEnt);
        if (!pTrigger)
        {
            AssertMsg(false, "Entity with classname trigger_momentum_timer_* was not a momentum zone trigger");
            continue;
        }

        const auto trackNum = pTrigger->GetTrackNumber();
        const auto zoneNum = pTrigger->GetZoneNumber();

        AssertMsg(trackNum >= -1 && trackNum < MAX_TRACKS, "Track number %i out of range!", trackNum);
        AssertMsg(zoneNum >= 0 && zoneNum < MAX_ZONES, "Zone number %i out of range!", zoneNum);

        if (trackNum > -1)
            trackTriggers[trackNum][zoneNum] = pTrigger;
        else if (trackNum == -1)
            versatileTriggers.AddToTail(pTrigger);

        pEnt = gEntList.FindEntityByClassname(pEnt, "trigger_momentum_timer_*");
    }

    for (int track = -1; track < MAX_TRACKS; track++)
    {
        KeyValues *pKvTrack = new KeyValues(CFmtStr("%i", track));
        if (track == -1)
        {
            // Add our versatile triggers (track num -1)
            if (!versatileTriggers.IsEmpty())
            {
                FOR_EACH_VEC(versatileTriggers, i)
                {
                    const auto pZoneTrigger = versatileTriggers[i];
                    if (pZoneTrigger)
                        SaveTrigger(pZoneTrigger, pKvTrack);
                }
            }
        }
        else
        {
            // Go through each zone and find our triggers
            for (int zone = 0; zone < MAX_ZONES; zone++)
            {
                const auto pZoneTrigger = trackTriggers[track][zone];
                if (pZoneTrigger)
                    SaveTrigger(pZoneTrigger, pKvTrack);
            }
        }
        
        // Save only if we're not empty
        if (!pKvTrack->IsEmpty())
        {
            zoneKV->AddSubKey(pKvTrack);
        }
        else
        {
            pKvTrack->deleteThis();
        }
    }

    if (!zoneKV->IsEmpty())
    {
        char zoneFilePath[MAX_PATH];
        V_ComposeFileName(MAP_FOLDER, szMapName, zoneFilePath, MAX_PATH);
        V_SetExtension(zoneFilePath, EXT_ZONE_FILE, MAX_PATH);
        zoneKV->SaveToFile(filesystem, zoneFilePath, "MOD");
    }
}

CMapzoneData::CMapzoneData(const char *szMapName)
{
    if (!LoadFromFile(szMapName))
    {
        DevLog("No existing .zon file found!\n");
    }
}

CMapzoneData::~CMapzoneData()
{
    if (!m_zones.IsEmpty())
    {
        m_zones.PurgeAndDeleteElements();
    }
}

void CMapzoneData::SpawnMapZones()
{
    int count = m_zones.Count();
    for (int i = 0; i < count; i++)
    {
        m_zones[i]->SpawnZone();
    }
}

bool CMapzoneData::LoadFromFile(const char *szMapName)
{
    char zoneFilePath[MAX_PATH];
    V_ComposeFileName(MAP_FOLDER, szMapName, zoneFilePath, MAX_PATH);
    V_SetExtension(zoneFilePath, EXT_ZONE_FILE, MAX_PATH);
    DevLog("Looking for zone file: %s \n", zoneFilePath);

    KeyValuesAD fileKV(szMapName);
    if (fileKV->LoadFromFile(filesystem, zoneFilePath, "GAME"))
    {
        FOR_EACH_SUBKEY(fileKV, trackKV)
        {
            const auto trackNum = Q_atoi(trackKV->GetName());
            FOR_EACH_SUBKEY(trackKV, zoneKV)
            {
                // Load position information (will default to 0 if the keys don't exist)
                int zoneType = ZONE_TYPE_INVALID;

                const char *name = zoneKV->GetName();
                if (FStrEq(name, "start"))
                {
                    zoneType = ZONE_TYPE_START;
                }
                else if (FStrEq(name, "checkpoint"))
                {
                    zoneType = ZONE_TYPE_CHECKPOINT;
                }
                else if (FStrEq(name, "end"))
                {
                    zoneType = ZONE_TYPE_STOP;
                }
                else if (FStrEq(name, "stage"))
                {
                    zoneType = ZONE_TYPE_STAGE;
                }
                else
                {
                    Warning("Error while reading zone file: Unknown mapzone type %s!\n", zoneKV->GetName());
                    continue;
                }

                // Add element
                m_zones.AddToTail(new CMapZone(zoneType, trackNum, zoneKV));
            }
        }

        DevLog("Successfully loaded map zone file %s!\n", zoneFilePath);
        return true;
    }
    return false;
}

bool ZoneTypeToClass(int type, char *dest, int maxlen)
{
    switch (type)
    {
    case ZONE_TYPE_START:
        Q_strncpy(dest, "trigger_momentum_timer_start", maxlen);
        return true;
    case ZONE_TYPE_CHECKPOINT:
        Q_strncpy(dest, "trigger_momentum_timer_checkpoint", maxlen);
        return true;
    case ZONE_TYPE_STOP:
        Q_strncpy(dest, "trigger_momentum_timer_stop", maxlen);
        return true;
    case ZONE_TYPE_STAGE:
        Q_strncpy(dest, "trigger_momentum_timer_stage", maxlen);
        return true;
    default:
        return false;
    }
}