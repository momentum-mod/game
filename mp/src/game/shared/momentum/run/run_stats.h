#pragma once

#include "mom_shareddefs.h"
#include "util/serialization.h"

#ifdef CLIENT_DLL
#define CMomRunStats C_MomRunStats
EXTERN_RECV_TABLE(DT_MomRunStats);
#else
EXTERN_SEND_TABLE(DT_MomRunStats);
#endif

// Run stats collected throughout a run of a track.
class CMomRunStats : public ISerializable
{
public:
    DECLARE_CLASS_NOBASE(CMomRunStats);
    DECLARE_EMBEDDED_NETWORKVAR();

    // struct data;
    CMomRunStats(uint8 size = MAX_ZONES);
    CMomRunStats(CUtlBuffer &reader);
    virtual ~CMomRunStats() {}

    // Note: This needs updating every time the struct is updated!
    void Init(uint8 size = MAX_ZONES);

    // Note: This needs updating every time the struct is updated!
    void Deserialize(CUtlBuffer &reader);

    // Note: This needs updating every time the struct is updated!
    void Serialize(CUtlBuffer &writer) OVERRIDE;

    // Note: This needs updating every time the struct is updated!
    void FullyCopyFrom(const CMomRunStats &other);

    // All these are virtual so they can be overridden in future versions.
    uint8 GetTotalZones();
    uint32 GetZoneJumps(int zone);
    uint32 GetZoneStrafes(int zone);
    int GetZoneTicks(int zone);
    int GetZoneEnterTick(int zone);
    float GetZoneStrafeSyncAvg(int zone);
    float GetZoneStrafeSync2Avg(int zone);
    float GetZoneEnterSpeed(int zone, bool vel2D);
    float GetZoneExitSpeed(int zone, bool vel2D);
    float GetZoneVelocityMax(int zone, bool vel2D);
    float GetZoneVelocityAvg(int zone, bool vel2D);


    void SetTotalZones(uint8 zones);
    void SetZoneJumps(int zone, uint32 value);
    void SetZoneStrafes(int zone, uint32 value);
    void SetZoneTicks(int zone, int value);
    void SetZoneEnterTick(int zone, int value);
    void SetZoneStrafeSyncAvg(int zone, float value);
    void SetZoneStrafeSync2Avg(int zone, float value);
    void SetZoneEnterSpeed(int zone, float vert, float hor);
    void SetZoneVelocityMax(int zone, float vert, float hor);
    void SetZoneVelocityAvg(int zone, float vert, float hor);
    void SetZoneExitSpeed(int zone, float vert, float hor);

    CNetworkVar(uint8, m_iTotalZones);
    CNetworkArray(int, m_iZoneJumps, MAX_ZONES + 1);
    CNetworkArray(int, m_iZoneStrafes, MAX_ZONES + 1);
    CNetworkArray(int, m_iZoneTicks, MAX_ZONES + 1);
    CNetworkArray(int, m_iZoneEnterTick, MAX_ZONES + 1);
    CNetworkArray(float, m_flZoneStrafeSyncAvg, MAX_ZONES + 1);
    CNetworkArray(float, m_flZoneStrafeSync2Avg, MAX_ZONES + 1);
    CNetworkArray(float, m_flZoneEnterSpeed3D, MAX_ZONES + 1);
    CNetworkArray(float, m_flZoneEnterSpeed2D, MAX_ZONES + 1);
    CNetworkArray(float, m_flZoneVelocityMax3D, MAX_ZONES + 1);
    CNetworkArray(float, m_flZoneVelocityMax2D, MAX_ZONES + 1);
    CNetworkArray(float, m_flZoneVelocityAvg3D, MAX_ZONES + 1);
    CNetworkArray(float, m_flZoneVelocityAvg2D, MAX_ZONES + 1);
    CNetworkArray(float, m_flZoneExitSpeed3D, MAX_ZONES + 1);
    CNetworkArray(float, m_flZoneExitSpeed2D, MAX_ZONES + 1);
};