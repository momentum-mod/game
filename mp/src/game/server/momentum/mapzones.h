#pragma once

#include "mapzones_edit.h"
#include "mom_shareddefs.h"

class CMapZone;
class CTriggerZone;

class CMapZoneSystem : public CAutoGameSystemPerFrame
{
public:
    CMapZoneSystem();

    void LevelInitPreEntity() OVERRIDE;
    void LevelInitPostEntity() OVERRIDE;
    void LevelShutdownPreEntity() OVERRIDE;
    void LevelShutdownPostEntity() OVERRIDE;
    void FrameUpdatePostEntityThink() OVERRIDE;

    void PostInit() OVERRIDE;

    bool ZoneTypeToClass(int type, char *dest, int maxlen);

    void ClearMapZones();
    void LoadZonesFromSite(KeyValues *pKvTracks, CBaseEntity *pEnt);
    void LoadZonesFromFile();
    bool LoadZonesFromKeyValues(KeyValues *pKvTracks, bool bFromSite);
    void SaveZoneTrigger(CBaseMomZoneTrigger *pZoneTrigger, KeyValues *pKvInto);
    void SaveZonesToFile();

    CMapZoneEdit *GetZoneEditor() { return &m_Editor; }

    // Calculates the stage count
    void CalculateZoneCounts();
    // Gets the total stage count
    int GetZoneCount(int track) const { return (track >= 0 && track < MAX_TRACKS) ? m_iZoneCount[track] : -1; }

    // Dispatch to player
    void DispatchMapInfo(CMomentumPlayer *pPlayer) const;

private:
    void ResetCounts();

    bool m_bLoadedFromSite;
    CMapZoneEdit m_Editor;
    CUtlVector<CMapZone*> m_Zones;

    // The number of zones for a given track
    int m_iZoneCount[MAX_TRACKS];
    uint64 m_iLinearTracks; // Bit-encoded, if (m_iLinearTracks & (1 << trackNum) > 0), it's linear
    int m_iHighestTrackNum; // Highest track number on this map
};

extern CMapZoneSystem g_MapZoneSystem;