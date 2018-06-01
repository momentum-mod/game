#pragma once

#include "cbase.h"

struct PositionPacket_t;
struct DecalPacket_t;
struct SavelocReqPacket_t;
struct ghostAppearance_t;
class CMomentumPlayer;
class CMomentumOnlineGhostEntity;

class CMomentumGhostClient : public CAutoGameSystemPerFrame
{
public:
    CMomentumGhostClient(const char *pName);

    //bool Init() OVERRIDE; MOM_TODO: Set state variables here?
    void PostInit() OVERRIDE;
    //void LevelInitPreEntity() OVERRIDE;
    void LevelInitPostEntity() OVERRIDE;
    void LevelShutdownPostEntity() OVERRIDE;
    void LevelShutdownPreEntity() OVERRIDE;
    void FrameUpdatePreEntityThink() OVERRIDE;
    void Shutdown() OVERRIDE;
    // MOM_TODO uncomment this for server STEAM_CALLBACK(, HandleFriendJoin, GameRichPresenceJoinRequested_t); // Joining from a friend's "JOIN GAME" option from steam

    void ClearCurrentGhosts(bool);

    void SendChatMessage(char *pMessage); // Sent from the player, who is trying to say a message to either a server or the lobby
    void ResetOtherAppearanceData(); // Resets every ghost's appearance data, mostly done when overrides are toggled, to apply them
    void SendAppearanceData(ghostAppearance_t appearance);
    void SetSpectatorTarget(CSteamID target, bool bStartedSpectating, bool bLeft = false);
    void SendDecalPacket(DecalPacket_t *packet);
    void SendSavelocReqPacket(CSteamID &target, SavelocReqPacket_t *packet);

    CMomentumOnlineGhostEntity *GetOnlineGhostEntityFromID(const CSteamID &id) { return GetOnlineGhostEntityFromID(id.ConvertToUint64()); }
    CMomentumOnlineGhostEntity *GetOnlineGhostEntityFromID(const uint64 &id);

    CUtlMap<uint64, CMomentumOnlineGhostEntity*> *GetOnlineGhostMap();

    static bool CreateNewNetFrame(PositionPacket_t &frame);

    static CMomentumPlayer *m_pPlayer;
private:
    ConVarRef m_cvarHostTimescale;
};

extern CMomentumGhostClient *g_pMomentumGhostClient;
