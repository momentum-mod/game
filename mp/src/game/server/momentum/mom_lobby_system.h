#pragma once

#include "mom_shareddefs.h"

class MomentumPacket;
class DecalPacket;
class SavelocReqPacket;
struct AppearanceData_t;
class CMomentumOnlineGhostEntity;

class CMomentumLobbySystem
{
public:
    CMomentumLobbySystem();
    ~CMomentumLobbySystem();

    void CallResult_LobbyCreated(LobbyCreated_t *pCreated, bool IOFailure);
    void CallResult_LobbyJoined(LobbyEnter_t *pEntered, bool IOFailure);

    void StartLobby();
    void LeaveLobby() const;
    bool TryJoinLobby(const CSteamID &lobbyID);
    bool TryJoinLobbyFromString(const char *pString);

    void ResetOtherAppearanceData(); // Sent when the player changes an override appearance cvar
    bool SendSavelocReqPacket(CSteamID& target, SavelocReqPacket *p);
    void TeleportToLobbyMember(const char *pIDStr);

    STEAM_CALLBACK(CMomentumLobbySystem, HandleLobbyEnter, LobbyEnter_t); // We entered this lobby (or failed to enter)
    STEAM_CALLBACK(CMomentumLobbySystem, HandleLobbyChatUpdate, LobbyChatUpdate_t); // Lobby chat room status has changed. This can be owner being changed, or somebody joining or leaving
    STEAM_CALLBACK(CMomentumLobbySystem, HandleLobbyDataUpdate, LobbyDataUpdate_t); // Something was updated for the lobby's data
    STEAM_CALLBACK(CMomentumLobbySystem, HandleLobbyJoin, GameLobbyJoinRequested_t); // We are trying to join a lobby
    STEAM_CALLBACK(CMomentumLobbySystem, HandleNewP2PRequest, SteamNetworkingMessagesSessionRequest_t); // Somebody is trying to talk to us
    STEAM_CALLBACK(CMomentumLobbySystem, HandleP2PConnectionFail, SteamNetworkingMessagesSessionFailed_t); // Talking/connecting to somebody failed
    STEAM_CALLBACK(CMomentumLobbySystem, HandlePersonaCallback, PersonaStateChange_t); // Called when we get their avatar and name from steam

    // Deprecated??
    STEAM_CALLBACK(CMomentumLobbySystem, HandleNewP2PRequest_OLD, P2PSessionRequest_t); // Somebody is trying to talk to us
    STEAM_CALLBACK(CMomentumLobbySystem, HandleP2PConnectionFail_OLD, P2PSessionConnectFail_t); // Talking/connecting to somebody failed

    static CSteamID m_sLobbyID;
    static float m_flNextUpdateTime;

    static CSteamID GetLobbyId() { return m_sLobbyID; }
    static bool LobbyValid() { return m_sLobbyID.IsValid() && m_sLobbyID.IsLobby(); }

    void LevelChange(const char *pMapName); // This client has changed levels to (potentially) a different map

    void CreateLobbyGhostEntity(const CSteamID &lobbyMember);
    void CreateLobbyGhostEntities(); // Creates everyone's ghosts if possible

    void SendAndReceiveP2PPackets();

    void SetSpectatorTarget(const CSteamID &ghostTarget, bool bStarted, bool bLeft = false);
    void SetIsSpectating(bool bSpec);
    bool GetIsSpectatingFromMemberData(const CSteamID &who);
    uint64 GetSpectatingTargetFromMemberData(const CSteamID &person);

    bool SendDecalPacket(DecalPacket *packet);

    void OnLobbyMaxPlayersChanged(int newMax);
    void OnLobbyTypeChanged(int newType);

    void SetAppearanceInMemberData(const AppearanceData_t &appearance);
    bool GetAppearanceFromMemberData(const CSteamID &member, AppearanceData_t &out);

    CMomentumOnlineGhostEntity *GetLobbyMemberEntity(const CSteamID &id) { return GetLobbyMemberEntity(id.ConvertToUint64()); }
    CMomentumOnlineGhostEntity *GetLobbyMemberEntity(const uint64 &id);

    void ClearCurrentGhosts(bool bLeavingLobby); // Clears the current ghosts stored in the map

    CUtlMap<uint64, CMomentumOnlineGhostEntity*> *GetOnlineEntMap() { return &m_mapLobbyGhosts;}

private:
    CUtlVector<CSteamID> m_vecBlocked; // Vector of blocked users (ignore updates/packets from these people)

    CUtlMap<uint64, CMomentumOnlineGhostEntity*> m_mapLobbyGhosts;

    bool m_bHostingLobby;

    void HandleNewP2PRequestInternal(const SteamNetworkingIdentity &identity);
    void HandleP2PConnectionFailInternal(const SteamNetworkingIdentity &identity);

    // Sends a packet to a specific person
    bool SendPacket(MomentumPacket *packet, const CSteamID &target, int sendType = k_nSteamNetworkingSend_Unreliable) const;
    bool SendPacketToEveryone(MomentumPacket *pPacket, int sendType = k_nSteamNetworkingSend_Unreliable);

    void WriteLobbyMessage(LobbyMessageType_t type, uint64 id);
    void WriteSpecMessage(SpectateMessageType_t type, uint64 playerID, uint64 targetID);

    bool IsInSameMapAs(const CSteamID &other);
    bool IsInLobby(const CSteamID &other);
    bool IsUserBlocked(const CSteamID &other);

    void UpdateCurrentLobbyMap(const char *pMapName);
    void UpdateLobbyOwner();

    void UpdateLobbyEntityFromMemberData(CMomentumOnlineGhostEntity *pEntity);
    void OnLobbyMemberDataChanged(const CSteamID &memberID);

    // When the lobby member leaves either the map or the lobby
    void OnLobbyMemberLeave(const CSteamID &member);

    CCallResult<CMomentumLobbySystem, LobbyCreated_t> m_cLobbyCreated;
    CCallResult<CMomentumLobbySystem, LobbyEnter_t> m_cLobbyJoined;
};

extern CMomentumLobbySystem *g_pMomentumLobbySystem;