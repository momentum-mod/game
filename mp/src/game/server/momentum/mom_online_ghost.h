#pragma once

#include "mom_ghost_base.h"
#include "utlqueue.h"
#include "GameEventListener.h"

class CMomentumOnlineGhostEntity : public CMomentumGhostBaseEntity, public CGameEventListener
{
    DECLARE_CLASS(CMomentumOnlineGhostEntity, CMomentumGhostBaseEntity)
    DECLARE_DATADESC();
    DECLARE_SERVERCLASS();

public:
    CMomentumOnlineGhostEntity();
    ~CMomentumOnlineGhostEntity();

    // Adds a position frame to the queue for processing
    void AddPositionFrame(const PositionPacket &newFrame);
    // Adds a decal frame to the queue of processing
    // Note: We have to delay the decal packets to sort of sync up to position, to make spectating more accurate.
    void AddDecalFrame(const DecalPacket &decal);
    // Places a decal in the world, according to the packet and decal type
    void FireDecal(const DecalPacket &decal);

    void SetGhostSteamID(const CSteamID &steamID);
    CSteamID GetGhostSteamID() const { return m_GhostSteamID; }
    void SetGhostName(const char *pGhostName);

    void AppearanceFlashlightChanged(const AppearanceData_t &newApp) override;
    void AppearanceModelColorChanged(const AppearanceData_t &newApp) override;

    bool IsOnlineGhost() const OVERRIDE { return true; }

    void SetGhostFlashlight(bool bEnable);

    void Spawn() OVERRIDE;
    void HandleGhost() OVERRIDE;
    void HandleGhostFirstPerson() OVERRIDE;
    void UpdateStats(const Vector &ghostVel) OVERRIDE; // for hud display..

    // Fills the current data, returns false if it couldn't
    bool GetCurrentPositionPacketData(PositionPacket *out) const;

    void UpdatePlayerSpectate();

    CNetworkVar(uint32, m_uiAccountID);
    CNetworkVar(bool, m_bSpectating);

    IMPLEMENT_NETWORK_VAR_FOR_DERIVED(m_vecViewOffset);

    QAngle m_vecLookAngles; // Used for storage reasons

protected:
    void CreateTrail() OVERRIDE;

    void Think() OVERRIDE;
    void FireGameEvent(IGameEvent *pEvent) OVERRIDE;

private:
    void DoPaint(const DecalPacket &packet);
    void DoKnifeSlash(const DecalPacket &packet);
    void ThrowGrenade(const DecalPacket &packet);
    void FireRocket(const DecalPacket &packet);
    void FireSticky(const DecalPacket &packet);
    void DetonateStickies();

    CUtlQueue<ReceivedFrame_t<PositionPacket>*> m_vecPositionPackets;
    ReceivedFrame_t<PositionPacket>* m_pCurrentFrame;
    ReceivedFrame_t<PositionPacket>* m_pNextFrame;
    CUtlQueue<ReceivedFrame_t<DecalPacket>*> m_vecDecalPackets;

    CSteamID m_GhostSteamID;
};