#pragma once
#include "cbase.h"
#include "mom_ghostdefs.h"

class CMomentumOnlineGhostEntity;

class CMomentumLobbySystem
{
public:
    CMomentumLobbySystem();
    ~CMomentumLobbySystem();

    void CallResult_LobbyCreated(LobbyCreated_t *pCreated, bool IOFailure);
    void CallResult_LobbyJoined(LobbyEnter_t *pEntered, bool IOFailure);

    void StartLobby();
    void LeaveLobby();
    void JoinLobbyFromString(const char *pString);

    void SendChatMessage(char *pMessage); // Sent from the player, who is trying to say a message
    void ResetOtherAppearanceData(); // Sent when the player changes an override appearance cvar

    STEAM_CALLBACK(CMomentumLobbySystem, HandleLobbyEnter, LobbyEnter_t); // We entered this lobby (or failed to enter)
    STEAM_CALLBACK(CMomentumLobbySystem, HandleLobbyChatUpdate, LobbyChatUpdate_t); // Lobby chat room status has changed. This can be owner being changed, or somebody joining or leaving
    STEAM_CALLBACK(CMomentumLobbySystem, HandleLobbyChatMsg, LobbyChatMsg_t); // Lobby chat message sent here, used with SayText etc
    STEAM_CALLBACK(CMomentumLobbySystem, HandleLobbyDataUpdate, LobbyDataUpdate_t); // Something was updated for the lobby's data
    STEAM_CALLBACK(CMomentumLobbySystem, HandleLobbyJoin, GameLobbyJoinRequested_t); // We are trying to join a lobby
    STEAM_CALLBACK(CMomentumLobbySystem, HandleNewP2PRequest, P2PSessionRequest_t); // Somebody is trying to talk to us
    STEAM_CALLBACK(CMomentumLobbySystem, HandleP2PConnectionFail, P2PSessionConnectFail_t); // Talking/connecting to somebody failed
    STEAM_CALLBACK(CMomentumLobbySystem, HandlePersonaCallback, PersonaStateChange_t); // Called when we get their avatar and name from steam

    static CSteamID m_sLobbyID;
    static float m_flNextUpdateTime;

    static CSteamID GetLobbyId();
    static bool LobbyValid() { return m_sLobbyID.IsValid() && m_sLobbyID.IsLobby(); }

    void LevelChange(const char *pMapName); // This client has changed levels to (potentially) a different map
    void CheckToAdd(CSteamID *pID);

    void SendAndRecieveP2PPackets();
    void SetAppearanceInMemberData(ghostAppearance_t app);
    void SetSpectatorTarget(const CSteamID &ghostTarget, bool bStarted);
    void SetIsSpectating(bool bSpec);
    void SendSpectatorUpdatePacket(const CSteamID &ghostTarget, SPECTATE_MSG_TYPE type);
    bool GetIsSpectatingFromMemberData(const CSteamID &who);
    void SendDecalPacket(DecalPacket_t *packet);

    void SetGameInfoStatus();
    LobbyGhostAppearance_t GetAppearanceFromMemberData(const CSteamID &member);

    CMomentumOnlineGhostEntity *GetLobbyMemberEntity(const CSteamID &id) { return GetLobbyMemberEntity(id.ConvertToUint64()); }
    CMomentumOnlineGhostEntity *GetLobbyMemberEntity(const uint64 &id);

    void ClearCurrentGhosts(bool bRemoveEnts); // Clears the current ghosts stored in the map

private:
    CUtlVector<CSteamID> m_vecBlocked; // Vector of blocked users (ignore updates/packets from these people)

    CUtlMap<uint64, CMomentumOnlineGhostEntity*> m_mapLobbyGhosts;

    bool m_bHostingLobby;

    // Sends a packet to a specific person, or everybody (if pTarget is null)
    void SendPacket(MomentumPacket_t *packet, CSteamID *pTarget = nullptr, EP2PSend sendType = k_EP2PSendUnreliable);

    void WriteMessage(LOBBY_MSG_TYPE type, uint64 id);
    void WriteMessage(SPECTATE_MSG_TYPE type, uint64 playerID, uint64 ghostID);

    CCallResult<CMomentumLobbySystem, LobbyCreated_t> m_cLobbyCreated;
    CCallResult<CMomentumLobbySystem, LobbyEnter_t> m_cLobbyJoined;
};

extern CMomentumLobbySystem *g_pMomentumLobbySystem;
