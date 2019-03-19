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
    void AddPositionFrame(const PositionPacket_t &newFrame);
    // Adds a decal frame to the queue of processing
    // Note: We have to delay the decal packets to sort of sync up to position, to make spectating more accurate.
    void AddDecalFrame(const DecalPacket_t &decal);
    // Places a decal in the world, according to the packet and decal type
    void FireDecal(const DecalPacket_t &decal);

    void SetGhostSteamID(const CSteamID &steamID)
    {
        m_GhostSteamID = steamID;
        m_uiAccountID = m_GhostSteamID.ConvertToUint64();
    }
    CSteamID GetGhostSteamID() const { return m_GhostSteamID; }
    void SetGhostName(const char *pGhostName);

    void SetLobbyGhostAppearance(LobbyGhostAppearance_t app, bool bForceUpdate = false);
    LobbyGhostAppearance_t GetLobbyGhostAppearance() const { return m_CurrentAppearance; }

    bool IsOnlineGhost() const OVERRIDE { return true; }

    void Spawn() OVERRIDE;
    void HandleGhost() OVERRIDE;
    void HandleGhostFirstPerson() OVERRIDE;
    void UpdateStats(const Vector &ghostVel) OVERRIDE; // for hud display..

    void UpdatePlayerSpectate();

    CNetworkVar(uint32, m_uiAccountID);
    CNetworkVar(bool, m_bSpectating);

    QAngle m_vecLookAngles; // Used for storage reasons

protected:
    void Think(void) OVERRIDE;
    void Precache(void) OVERRIDE;
    void FireGameEvent(IGameEvent *pEvent) OVERRIDE;

private:
    void DoPaint(const DecalPacket_t &packet);
    void DoKnifeSlash(const DecalPacket_t &packet);
    void ThrowGrenade(const DecalPacket_t &packet);

    CUtlQueue<ReceivedFrame_t<PositionPacket_t>*> m_vecPositionPackets;
    ReceivedFrame_t<PositionPacket_t>* m_pCurrentFrame;
    ReceivedFrame_t<PositionPacket_t>* m_pNextFrame;
    CUtlQueue<ReceivedFrame_t<DecalPacket_t>*> m_vecDecalPackets;

    CSteamID m_GhostSteamID;
    LobbyGhostAppearance_t m_CurrentAppearance;
};
