#pragma once

#include "cbase.h"
#include "mom_player.h"
#include "mom_shareddefs.h"
#include "mom_online_ghost.h"

struct MyThreadParams_t{}; //empty class so we can force the threaded function to work xd

class CMomentumGhostClient : public CAutoGameSystemPerFrame
{
public:
    CMomentumGhostClient(const char *pName) : CAutoGameSystemPerFrame(pName), 
        m_sHostID(), m_bHostingLobby(false), m_bRanIOThread(false)
    {
        SetDefLessFunc(m_mapOnlineGhosts);
        m_pInstance = this;
    }

    //bool Init() OVERRIDE; MOM_TODO: Set state variables here?
    void PostInit() OVERRIDE;
    //void LevelInitPreEntity() OVERRIDE;
    void LevelInitPostEntity() OVERRIDE;
    //void LevelShutdownPreEntity() OVERRIDE;
    void LevelShutdownPreEntity() OVERRIDE;
    void FrameUpdatePreEntityThink() OVERRIDE;
    void Shutdown() OVERRIDE;

    void CallResult_LobbyCreated(LobbyCreated_t *pCreated, bool IOFailure);
    void CallResult_LobbyJoined(LobbyEnter_t *pEntered, bool IOFailure);

    void StartLobby();
    void LeaveLobby();

    STEAM_CALLBACK(CMomentumGhostClient, HandleLobbyEnter, LobbyEnter_t); // We entered this lobby (or failed to enter)
    STEAM_CALLBACK(CMomentumGhostClient, HandleLobbyChatUpdate, LobbyChatUpdate_t); // Lobby chat room status has changed. This can be owner being changed, or somebody joining or leaving
    STEAM_CALLBACK(CMomentumGhostClient, HandleLobbyChatMsg, LobbyChatMsg_t); // Lobby chat message sent here, used with SayText etc
    STEAM_CALLBACK(CMomentumGhostClient, HandleLobbyDataUpdate, LobbyDataUpdate_t); // Something was updated for the lobby's data
    STEAM_CALLBACK(CMomentumGhostClient, HandleLobbyJoin, GameLobbyJoinRequested_t); // We are trying to join a lobby
    STEAM_CALLBACK(CMomentumGhostClient, HandleFriendJoin, GameRichPresenceJoinRequested_t); // Joining from a friend's "JOIN GAME" option from steam
    STEAM_CALLBACK(CMomentumGhostClient, HandleNewP2PRequest, P2PSessionRequest_t); // Somebody is trying to talk to us
    STEAM_CALLBACK(CMomentumGhostClient, HandleP2PConnectionFail, P2PSessionConnectFail_t); // Talking/connecting to somebody failed
    STEAM_CALLBACK(CMomentumGhostClient, HandlePersonaCallback, PersonaStateChange_t); // Called when we get their avatar and name from steam

    void SendChatMessage(char *pMessage); // Sent from the player, who is trying to say a message to either a server or the lobby
    void NotifyStartTyping(CSteamID pMember);
    void NotifyStopTyping(CSteamID pMember);
    void GetLobbyMemberSteamData(CSteamID pMember);

    static unsigned SendAndRecieveP2PPackets(void *args);
    static ghostNetFrame_t CreateNewNetFrame(CMomentumPlayer *pPlayer);
    static ghostAppearance_t CreateAppearance(CMomentumPlayer* pPlayer) { return pPlayer->m_playerAppearanceProps; }

    void JoinLobbyFromString(const char *pString);
private:

    //static CThreadMutex m_mtxGhostPlayers, m_mtxpPlayer;
    static CMomentumPlayer *m_pPlayer;
    //static uint64 m_SteamID;
    //static ghostAppearance_t oldAppearance;

    static CUtlMap<uint64, CMomentumOnlineGhostEntity*> m_mapOnlineGhosts;

    void CheckToAdd(CSteamID *pSteamID);
    void ClearCurrentGhosts();

    static CSteamID m_sLobbyID;

    CSteamID m_sHostID;

    bool m_bHostingLobby;
    bool m_bRanIOThread;
    static float m_flNextUpdateTime; //needs to be static due to thread..
    static bool m_bIsLobbyValid;
    CCallResult<CMomentumGhostClient, LobbyCreated_t> m_cLobbyCreated;
    CCallResult<CMomentumGhostClient, LobbyEnter_t> m_cLobbyJoined;
    static CMomentumGhostClient *m_pInstance;
};

extern CMomentumGhostClient *g_pMomentumGhostClient;
