#include "cbase.h"
#include "filesystem.h"
#include "mapzones.h"
#include "mom_player.h"
#include "mom_triggers.h"
#include "mapzones_build.h"
#include "fmtstr.h"

#include "tier0/memdbgon.h"

static void SaveZonFile(const char *pMapName);
CON_COMMAND(mom_zone_generate, "Generates the .zon file for map zones.")
{
    SaveZonFile(gpGlobals->mapname.ToCStr());
}

class CMapZone
{
  public:
    CMapZone(int track, int type, const KeyValues *values);
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

CMapZone::CMapZone(const int track, const int type, const KeyValues *values)
{
    m_iType = type;
    m_iTrack = track;
    m_pZoneValues = values->MakeCopy();
    m_pTrigger = nullptr;
}

CMapZone::~CMapZone()
{
    m_pZoneValues->deleteThis();
    m_pTrigger = nullptr;
}

void CMapZone::SpawnZone()
{
    char classname[64];
    g_MapZoneSystem.ZoneTypeToClass(m_iType, classname, sizeof(classname));
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
    KeyValuesAD zoneKV("tracks");
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

    if (!zoneKV->IsEmpty() && szMapName)
    {
        char zoneFilePath[MAX_PATH];
        V_ComposeFileName(MAP_FOLDER, szMapName, zoneFilePath, MAX_PATH);
        V_SetExtension(zoneFilePath, EXT_ZONE_FILE, MAX_PATH);
        zoneKV->SaveToFile(filesystem, zoneFilePath, "MOD");
    }
}

CMapZoneSystem::CMapZoneSystem() : CAutoGameSystemPerFrame("CMapZoneSystem"), m_iLinearTracks(0), m_iHighestTrackNum(0)
{
    m_bLoadedFromSite = false;
}

void CMapZoneSystem::LevelInitPreEntity()
{
    ClearMapZones();
}

void CMapZoneSystem::LevelInitPostEntity()
{
    m_Editor.LevelInit();
}

void CMapZoneSystem::LevelShutdownPreEntity()
{
    ClearMapZones();
    m_bLoadedFromSite = false;

    for (int i = 0; i < MAX_TRACKS; i++)
    {
        m_iZoneCount[i] = 0;
    }
    m_iLinearTracks = 0;
}

void CMapZoneSystem::LevelShutdownPostEntity()
{
    m_Editor.LevelShutdown();
}

void CMapZoneSystem::FrameUpdatePostEntityThink()
{
    m_Editor.FrameUpdate();
}

bool CMapZoneSystem::ZoneTypeToClass(int type, char *dest, int maxlen)
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

void CMapZoneSystem::ClearMapZones()
{
    if (!m_Zones.IsEmpty())
        m_Zones.PurgeAndDeleteElements();
}

void CMapZoneSystem::LoadZonesFromSite(KeyValues *pKvTracks, CBaseEntity *pEnt)
{
    if (LoadZonesFromKeyValues(pKvTracks, true))
    {
        m_bLoadedFromSite = true;
        const auto pPlayer = dynamic_cast<CMomentumPlayer*>(pEnt);
        if (pPlayer)
        {
            DispatchMapInfo(pPlayer);
        }
    }
}

void CMapZoneSystem::LoadZonesFromFile()
{
    m_bLoadedFromSite = false;
    char zoneFilePath[MAX_PATH];
    V_ComposeFileName(MAP_FOLDER, gpGlobals->mapname.ToCStr(), zoneFilePath, MAX_PATH);
    V_SetExtension(zoneFilePath, EXT_ZONE_FILE, MAX_PATH);
    DevLog("Looking for zone file: %s \n", zoneFilePath);

    KeyValuesAD fileKV("tracks");
    if (fileKV->LoadFromFile(filesystem, zoneFilePath, "GAME"))
    {
        const auto bSuccess = LoadZonesFromKeyValues(fileKV, false);
        DevLog("%s map zone file %s!\n", bSuccess ? "Successfully loaded" : "Failed to load", zoneFilePath);
    }
}

bool CMapZoneSystem::LoadZonesFromKeyValues(KeyValues *pKvTracks, bool bFromSite)
{
    if (!pKvTracks || pKvTracks->IsEmpty())
        return false;

    ResetCounts();
    int globalZones = 0;

    FOR_EACH_SUBKEY(pKvTracks, trackKV)
    {
        const auto trackNum = bFromSite ? trackKV->GetInt("trackNum") : Q_atoi(trackKV->GetName());
        if (trackNum >= -1 && trackNum < MAX_TRACKS)
        {
            KeyValues *toItr = bFromSite ? trackKV->FindKey("zones") : trackKV;
            FOR_EACH_SUBKEY(toItr, zoneKV)
            {
                const auto zoneNum = bFromSite ? zoneKV->GetInt("zoneNum") : Q_atoi(zoneKV->GetName());
                if (zoneNum >= 0 && zoneNum < MAX_ZONES)
                {
                    const auto zoneType = zoneKV->GetInt("zoneType", ZONE_TYPE_INVALID);

                    if (zoneType <= ZONE_TYPE_INVALID || zoneType >= ZONE_TYPE_COUNT)
                    {
                        Warning("Error while reading zone file: Unknown map zone type %d!\n", zoneType);
                        continue;
                    }

                    if (zoneType != ZONE_TYPE_STOP)
                    {
                        if (trackNum > -1 && trackNum < MAX_TRACKS)
                        {
                            if (trackNum > m_iHighestTrackNum)
                                m_iHighestTrackNum = trackNum;

                            m_iZoneCount[trackNum]++;
                            if (zoneType == ZONE_TYPE_CHECKPOINT)
                                m_iLinearTracks |= (1ULL << trackNum);
                        }
                        else if (trackNum == -1)
                            globalZones++;
                    }

                    // Add element
                    auto pMapZone = new CMapZone(trackNum, zoneType, zoneKV);
                    pMapZone->SpawnZone();
                    m_Zones.AddToTail(pMapZone);
                }
            }
        }
    }

    // Add in all the global zones, if we have any
    if (globalZones)
    {
        for (int i = 0; i <= m_iHighestTrackNum; i++)
        {
            m_iZoneCount[i] += globalZones;
        }
    }

    return !m_Zones.IsEmpty();
}

void CMapZoneSystem::CalculateZoneCounts(CMomentumPlayer *pDispatch)
{
    // Reset our counts
    ResetCounts();

    int globalZones = 0;
    auto pEnt = gEntList.FindEntityByClassname(nullptr, "trigger_momentum_timer_*");
    while (pEnt)
    {
        const auto pTrigger = dynamic_cast<CBaseMomZoneTrigger*>(pEnt);
        if (pTrigger)
        {
            const auto iZoneType = pTrigger->GetZoneType();
            if (iZoneType == ZONE_TYPE_START || iZoneType == ZONE_TYPE_STAGE || iZoneType == ZONE_TYPE_CHECKPOINT)
            {
                const int iTrack = pTrigger->GetTrackNumber();

                if (iTrack > -1 && iTrack < MAX_TRACKS)
                {
                    if (iTrack > m_iHighestTrackNum)
                        m_iHighestTrackNum = iTrack;

                    m_iZoneCount[iTrack]++;
                    if (iZoneType == ZONE_TYPE_CHECKPOINT)
                        m_iLinearTracks |= (1ULL << iTrack);
                }
                else if (iTrack == -1)
                    globalZones++;
            }
        }

        pEnt = gEntList.FindEntityByClassname(pEnt, "trigger_momentum_timer_*");
    }

    // Add in all the global zones, if we have any
    if (globalZones)
    {
        for (int i = 0; i <= m_iHighestTrackNum; i++)
        {
            m_iZoneCount[i] += globalZones;
        }
    }

    if (pDispatch)
        DispatchMapInfo(pDispatch);
}

void CMapZoneSystem::DispatchMapInfo(CMomentumPlayer *pPlayer) const
{
    // Copy over to the player
    for (uint64 i = 0; i <= m_iHighestTrackNum; i++)
    {
        pPlayer->m_iZoneCount.Set(i, m_iZoneCount[i]);
        pPlayer->m_iLinearTracks.Set(i, (m_iLinearTracks & (1ULL << i)) > 0);
    }
}

void CMapZoneSystem::DispatchNoZonesMsg(CMomentumPlayer *pPlayer) const
{
    if (m_iZoneCount[TRACK_MAIN] == 0)
    {
        CSingleUserRecipientFilter filter(pPlayer);
        filter.MakeReliable();
        UserMessageBegin(filter, "MB_NoStartOrEnd");
        MessageEnd();
    }
    else
    {
        DispatchMapInfo(pPlayer);
    }
}

void CMapZoneSystem::ResetCounts()
{
    for (auto i = 0; i < MAX_TRACKS; i++)
        m_iZoneCount[i] = 0;
    m_iLinearTracks = 0;
    m_iHighestTrackNum = -1;
}

CMapZoneSystem g_MapZoneSystem;
