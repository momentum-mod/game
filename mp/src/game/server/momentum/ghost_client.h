#pragma once

class PositionPacket;
class DecalPacket;
class SavelocReqPacket;
struct AppearanceData_t;
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

    void ClearCurrentGhosts(bool bLeavingLobby);

    void ResetOtherAppearanceData(); // Resets every ghost's appearance data, mostly done when overrides are toggled, to apply them
    void SendAppearanceData(const AppearanceData_t &appearance);
    void SetIsSpectating(bool state);
    void SetSpectatorTarget(CSteamID target, bool bStartedSpectating, bool bLeft = false);
    void SendDecalPacket(DecalPacket *packet);
    bool SendSavelocReqPacket(CSteamID &target, SavelocReqPacket *packet);

    bool IsInOnlineSession();

    CMomentumOnlineGhostEntity *GetOnlineGhostEntityFromID(const CSteamID &id) { return GetOnlineGhostEntityFromID(id.ConvertToUint64()); }
    CMomentumOnlineGhostEntity *GetOnlineGhostEntityFromID(const uint64 &id);

    CUtlMap<uint64, CMomentumOnlineGhostEntity*> *GetOnlineGhostMap();

    static bool CreateNewNetFrame(PositionPacket &frame);
};

extern CMomentumGhostClient *g_pMomentumGhostClient;
