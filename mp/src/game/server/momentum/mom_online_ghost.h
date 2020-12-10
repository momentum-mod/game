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

    void SetGhostName(const char *pGhostName);

    void AppearanceFlashlightChanged(const AppearanceData_t &newApp) override;
    void AppearanceModelColorChanged(const AppearanceData_t &newApp) override;

    bool IsOnlineGhost() const OVERRIDE { return true; }

    SpectateMessageType_t UpdateSpectateState(bool bIsSpec, uint64 specTargetID);
    void SetGhostFlashlight(bool bEnable);
    bool IsSpectating() const { return m_bSpectating.Get(); }
    uint64 GetSpecTarget() const { return m_specTargetID; }

    void Spawn() OVERRIDE;
    void HandleGhost() OVERRIDE;
    void HandleGhostFirstPerson() OVERRIDE;
    void UpdateStats(const Vector &ghostVel) OVERRIDE; // for hud display..

    // Fills the current data, returns false if it couldn't
    bool GetCurrentPositionPacketData(PositionPacket *out) const;

    void UpdatePlayerSpectate();

    IMPLEMENT_NETWORK_VAR_FOR_DERIVED(m_vecViewOffset);

    QAngle m_vecLookAngles; // Used for storage reasons

protected:
    void CreateTrail() OVERRIDE;

    void Think() OVERRIDE;
    void FireGameEvent(IGameEvent *pEvent) OVERRIDE;

private:
    CNetworkVar(bool, m_bSpectating);
    uint64 m_specTargetID;

    void DoPaint(const DecalPacket &packet);
    void DoKnifeSlash(const DecalPacket &packet);
    void ThrowGrenade(const DecalPacket &packet);
    void FireRocket(const DecalPacket &packet);
    void FireSticky(const DecalPacket &packet);
    void DetonateStickies();
    void ThrowConc(const DecalPacket &packet);

    void SetIsSpectating(bool bState);

    CUtlQueue<ReceivedFrame_t<PositionPacket>*> m_vecPositionPackets;
    ReceivedFrame_t<PositionPacket>* m_pCurrentFrame;
    ReceivedFrame_t<PositionPacket>* m_pNextFrame;
    CUtlQueue<ReceivedFrame_t<DecalPacket>*> m_vecDecalPackets;

    ConVarRef m_cvarPaintSound;
};
