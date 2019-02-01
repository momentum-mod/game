#ifndef MAPZONES_H
#define MAPZONES_H
#ifdef _WIN32
#pragma once
#endif

#include "filesystem.h"
#include "mom_triggers.h"

#define MOMZONETYPE_START       0
#define MOMZONETYPE_CP          1
#define MOMZONETYPE_STOP        2
#define MOMZONETYPE_ONEHOP      3
#define MOMZONETYPE_RESETONEHOP 4
#define MOMZONETYPE_CPTELE      5
#define MOMZONETYPE_MULTIHOP    6
#define MOMZONETYPE_STAGE       7

class CMapzone
{
public:
    CMapzone();
    CMapzone(const int, Vector, QAngle, Vector, Vector,
        const int, const bool, const bool, const float, 
        const bool, const float, const float,
        const string_t, const bool, 
        const int, const int, const int, const bool,
        const CUtlVector<Vector>&, const float);
    ~CMapzone(){};

    void SpawnZone();
    void RemoveZone();

    int GetType() { return m_iType; }
    Vector GetPosition() { return m_vecPos; }
    QAngle GetRotation() { return m_angRot; }
    Vector GetScaleMins() { return m_vecScaleMins; }
    Vector GetScaleMaxs() { return m_vecScaleMaxs; }

private:
    int m_iType; // Zone type, look above
    int m_index; // Ignored when not a checkpoint
    bool m_shouldStopOnTeleport; // Stop player on teleport?
    bool m_shouldResetAngles; // Reset the player's angles?
    float m_holdTimeBeforeTeleport; // How much to wait for before teleporting
    // startTrigger
    bool m_bLimitingSpeed; // Limit leave speed?
    bool m_bOnlyXYCheck; // Only checking speed in XY?
    bool m_bLimitBhop;
    float m_flMaxLeaveSpeed; // Max speed allowed
    float m_flBhopLeaveSpeed; // Max speed if player bhopped
    float m_flYaw; // Teleport yaw for start zone.
    string_t m_szLinkedEnt; // Entity name for teleporting to this entity (YESYES, It can be null!)
    Vector m_vecPos;
    QAngle m_angRot;
    Vector m_vecScaleMins;
    Vector m_vecScaleMaxs;
    CBaseEntity* m_pTrigger;
    CUtlVector<Vector> m_vZonePoints; 
    float m_flPointZoneHeight;
    int m_iEndZoneNumber, m_iStartZoneNumber;
    int m_iLimitType;
    bool m_bStartOnJump;
};

class CMapzoneData
{
public:
    CMapzoneData(const char *szMapName);
    ~CMapzoneData();

    
    void SpawnMapZones();
    void RemoveMapZones();
    bool MapZoneSpawned(CMapzone*);
    bool LoadFromFile(const char*);

private:
    CUtlVector<CMapzone*> m_zones;
};

bool ZoneTypeToClass(int type, char *dest);

#endif
