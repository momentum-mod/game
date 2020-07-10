#include "cbase.h"
#include "filesystem.h"
#include "mapzones.h"
#include "mom_player.h"
#include "mom_triggers.h"
#include "mapzones_build.h"
#include "fmtstr.h"

#include "tier0/memdbgon.h"

CON_COMMAND_F(mom_zone_generate, "Generates the .zon file for map zones.", FCVAR_MAPPING)
{
    g_MapZoneSystem.SaveZonesToFile();
}

class CMapZone
{
  public:
    CMapZone(int track, int zone, int type, const KeyValues *values);
    ~CMapZone();

    void SpawnZone();

    int GetType() const { return m_iType; }

  private:
    int m_iType;
    int m_iTrack; // Track number
    int m_iZone; // Zone number
    // KeyValues containing all the values describing the zone
    KeyValues *m_pZoneValues;

    CBaseMomZoneTrigger *m_pTrigger;
};

CMapZone::CMapZone(const int track, const int zone, const int type, const KeyValues *values)
{
    m_iType = type;
    m_iTrack = track;
    m_iZone = zone;
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
        m_pZoneValues->SetInt("zoneNum", m_iZone);
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
    CalculateZoneCounts();
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

void CMapZoneSystem::PostInit()
{
    filesystem->CreateDirHierarchy(ZONE_FOLDER, "MOD");
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
    case ZONE_TYPE_TRICK:
        Q_strncpy(dest, "trigger_momentum_trick", maxlen);
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
    V_ComposeFileName(ZONE_FOLDER, gpGlobals->mapname.ToCStr(), zoneFilePath, MAX_PATH);
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
            if (!toItr)
                return false;
            FOR_EACH_SUBKEY(toItr, zoneKV)
            {
                const auto zoneNum = zoneKV->GetInt("zoneNum");
                if (zoneNum >= 0 && zoneNum < MAX_ZONES)
                {
                    const auto pKvTriggers = zoneKV->FindKey("triggers");
                    if (!pKvTriggers)
                        return false;
                    FOR_EACH_SUBKEY(pKvTriggers, triggerKV)
                    {
                        const auto zoneType = triggerKV->GetInt("type", ZONE_TYPE_INVALID);

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

                                if (zoneNum > m_iZoneCount[trackNum])
                                    m_iZoneCount[trackNum] = zoneNum;

                                if (zoneType == ZONE_TYPE_CHECKPOINT)
                                    m_iLinearTracks |= (1ULL << trackNum);
                            }
                            else if (trackNum == -1)
                                globalZones++;
                        }

                        // Add element
                        auto pMapZone = new CMapZone(trackNum, zoneNum, zoneType, triggerKV);
                        pMapZone->SpawnZone();
                        m_Zones.AddToTail(pMapZone);
                    }
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

void CMapZoneSystem::SaveZoneTrigger(CBaseMomZoneTrigger *pZoneTrigger, KeyValues *pKvInto)
{
    bool bSuccess = false;
    const auto pKvTrigger = pKvInto->CreateNewKey();
    if (pZoneTrigger->ToKeyValues(pKvTrigger))
    {
        auto pBuilder = CreateZoneBuilderFromExisting(pZoneTrigger);

        bSuccess = pBuilder->Save(pKvTrigger);

        delete pBuilder;
    }

    if (!bSuccess)
    {
        Warning("Failed to save zone to file!\n");
        pKvInto->RemoveSubKey(pKvTrigger);
        pKvTrigger->deleteThis();
    }
}

struct MapZone
{
    CUtlVector<CTriggerZone*> m_Triggers;
};

struct MapTrack
{
    CUtlVector<MapZone*> m_Zones;
    ~MapTrack()
    {
        m_Zones.PurgeAndDeleteElements();
    }
};

template <class T>
inline void EnsureCount(T *vec, int count)
{
    if (vec->Count() < count)
    {
        auto diff = count - vec->Count();
        while (diff--)
        {
            vec->AddToTail(nullptr);
        }
    }
}

// May God have mercy on my soul
void CMapZoneSystem::SaveZonesToFile()
{
    CUtlVector<MapTrack*> vecTracks;
    CUtlVector<CTriggerZone*> versatileTriggers;
    int highestTrack = -1;
    int highestZoneForTrack[MAX_TRACKS];
    memset(highestZoneForTrack, 0, sizeof(highestZoneForTrack[0]) * MAX_TRACKS);
    
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
        {
            if (trackNum > highestTrack)
                highestTrack = trackNum;
            if (zoneNum > highestZoneForTrack[trackNum])
                highestZoneForTrack[trackNum] = zoneNum;

            if (trackNum < vecTracks.Count() && vecTracks[trackNum])
            {
                if (zoneNum < vecTracks[trackNum]->m_Zones.Count() && vecTracks[trackNum]->m_Zones[zoneNum])
                {
                    if (vecTracks[trackNum]->m_Zones[zoneNum]->m_Triggers.Count() < MAX_ZONE_TRIGGERS)
                        vecTracks[trackNum]->m_Zones[zoneNum]->m_Triggers.AddToTail(pTrigger);
                }
                else
                {
                    EnsureCount(&vecTracks[trackNum]->m_Zones, zoneNum + 1);
                    MapZone *pZone = new MapZone;
                    pZone->m_Triggers.AddToTail(pTrigger);
                    vecTracks[trackNum]->m_Zones[zoneNum] = pZone;
                }
            }
            else
            {
                EnsureCount(&vecTracks, trackNum+1);
                MapTrack *pTrack = new MapTrack;
                EnsureCount(&pTrack->m_Zones, zoneNum + 1);
                MapZone *pZone = new MapZone;
                pZone->m_Triggers.AddToTail(pTrigger);
                pTrack->m_Zones[zoneNum] = pZone;
                vecTracks[trackNum] = pTrack;
            }
        }
        else if (trackNum == -1)
            versatileTriggers.AddToTail(pTrigger);

        pEnt = gEntList.FindEntityByClassname(pEnt, "trigger_momentum_timer_*");
    }

    // Add our versatile triggers (track num -1)
    if (!versatileTriggers.IsEmpty())
    {
        FOR_EACH_VEC(versatileTriggers, versiIndx)
        {
            const auto pZoneTrigger = versatileTriggers[versiIndx];
            const auto pVersZoneNum = pZoneTrigger->GetZoneNumber();
            for (int t = 0; t <= highestTrack; t++)
            {
                // Only add this versatile trigger if its zone number fits within the zones for this track
                if (pVersZoneNum > -1 && pVersZoneNum <= highestZoneForTrack[t])
                {
                    if (vecTracks[t] && vecTracks[t]->m_Zones[pVersZoneNum] && 
                        vecTracks[t]->m_Zones[pVersZoneNum]->m_Triggers.Count() < MAX_ZONE_TRIGGERS)
                    {
                        vecTracks[t]->m_Zones[pVersZoneNum]->m_Triggers.AddToTail(pZoneTrigger);
                    }
                }
            }
        }
    }

    if (highestTrack > -1)
    {
        KeyValuesAD zoneKV("tracks");
        for (int track = 0; track <= highestTrack; track++)
        {
            const auto pKvTrack = new KeyValues(CFmtStr("%i", track));
            // Go through each zone and find our triggers
            for (int zone = 0; zone <= highestZoneForTrack[track]; zone++)
            {
                if (vecTracks[track] && vecTracks[track]->m_Zones[zone] && !vecTracks[track]->m_Zones[zone]->m_Triggers.IsEmpty())
                {
                    const auto pZoneKV = new KeyValues(CFmtStr("%i", zone));
                    pZoneKV->SetInt("zoneNum", zone);
                    const auto pTriggersKv = new KeyValues("triggers");
                    FOR_EACH_VEC(vecTracks[track]->m_Zones[zone]->m_Triggers, i)
                    {
                        SaveZoneTrigger(vecTracks[track]->m_Zones[zone]->m_Triggers[i], pTriggersKv);
                    }
                    pZoneKV->AddSubKey(pTriggersKv);
                    pKvTrack->AddSubKey(pZoneKV);
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

        if (!zoneKV->IsEmpty() && gpGlobals->mapname.ToCStr())
        {
            char zoneFilePath[MAX_PATH];
            V_ComposeFileName(ZONE_FOLDER, gpGlobals->mapname.ToCStr(), zoneFilePath, MAX_PATH);
            V_SetExtension(zoneFilePath, EXT_ZONE_FILE, MAX_PATH);
            zoneKV->SaveToFile(filesystem, zoneFilePath, "MOD");
        }

        vecTracks.PurgeAndDeleteElements();
    }
}

void CMapZoneSystem::CalculateZoneCounts()
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
                const auto pZoneTrigger = static_cast<CTriggerZone*>(pTrigger);

                if (iTrack > -1 && iTrack < MAX_TRACKS)
                {
                    if (iTrack > m_iHighestTrackNum)
                        m_iHighestTrackNum = iTrack;

                    if (pZoneTrigger->GetZoneNumber() > m_iZoneCount[iTrack])
                        m_iZoneCount[iTrack] = pZoneTrigger->GetZoneNumber();

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
}

void CMapZoneSystem::DispatchMapInfo(CMomentumPlayer *pPlayer) const
{
    const auto upper = m_iHighestTrackNum == -1 ? MAX_ZONES : m_iHighestTrackNum+1;

    // Copy over to the player
    for (uint64 i = 0; i < upper; i++)
    {
        pPlayer->m_iZoneCount.Set(i, m_iZoneCount[i]);
        pPlayer->m_iLinearTracks.Set(i, (m_iLinearTracks & (1ULL << i)) > 0);
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