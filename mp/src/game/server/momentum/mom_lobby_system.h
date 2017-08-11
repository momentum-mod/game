#pragma once
#include "cbase.h"
#include "mom_player.h"
#include "mom_shareddefs.h"
#include "ghost_client.h"

#define MEMBERDATA_MAP "map"
#define MEMBERDATA_APPS_BODYGROUP "appearance_bodygroup"
#define MEMBERDATA_APPS_MODELCOLOR "appearance_model_rgba"
#define MEMBERDATA_APPS_TRAILCOLOR "appearance_trail_rbga"
#define MEMBERDATA_APPS_TRAILLENGTH "appearance_trail_length"
#define MEMBERDATA_APPS_TRAILENABLE "appearance_trail_enable"
#define MEMBERDATA_APPS_MODEL "appearance_model_name"
class CMomentumLobbySystem
{
public:
    CMomentumLobbySystem(const char *pName) : m_bHostingLobby(false)
    {
        m_pInstance = this;
    }

    void CallResult_LobbyCreated(LobbyCreated_t *pCreated, bool IOFailure);
    void CallResult_LobbyJoined(LobbyEnter_t *pEntered, bool IOFailure);

    void StartLobby();
    void LeaveLobby();
    void JoinLobbyFromString(const char *pString);

    void SendChatMessage(char *pMessage); // Sent from the player, who is trying to say a message to either a server or the lobby
    void NotifyStartTyping(CSteamID pMember);
    void NotifyStopTyping(CSteamID pMember);
    void GetLobbyMemberSteamData(CSteamID pMember);

    STEAM_CALLBACK(CMomentumLobbySystem, HandleLobbyEnter, LobbyEnter_t); // We entered this lobby (or failed to enter)
    STEAM_CALLBACK(CMomentumLobbySystem, HandleLobbyChatUpdate, LobbyChatUpdate_t); // Lobby chat room status has changed. This can be owner being changed, or somebody joining or leaving
    STEAM_CALLBACK(CMomentumLobbySystem, HandleLobbyChatMsg, LobbyChatMsg_t); // Lobby chat message sent here, used with SayText etc
    STEAM_CALLBACK(CMomentumLobbySystem, HandleLobbyDataUpdate, LobbyDataUpdate_t); // Something was updated for the lobby's data
    STEAM_CALLBACK(CMomentumLobbySystem, HandleLobbyJoin, GameLobbyJoinRequested_t); // We are trying to join a lobby
    STEAM_CALLBACK(CMomentumLobbySystem, HandleFriendJoin, GameRichPresenceJoinRequested_t); // Joining from a friend's "JOIN GAME" option from steam
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
    void SetAppearanceInMemberData(CSteamID lobbyID, ghostAppearance_t app);
    ghostAppearance_t GetAppearanceFromMemberData(CSteamID lobbyID, CSteamID member);

private:

    bool m_bHostingLobby;

    CCallResult<CMomentumLobbySystem, LobbyCreated_t> m_cLobbyCreated;
    CCallResult<CMomentumLobbySystem, LobbyEnter_t> m_cLobbyJoined;
    static CMomentumLobbySystem *m_pInstance;

};

extern CMomentumLobbySystem *g_pMomentumLobbySystem;
