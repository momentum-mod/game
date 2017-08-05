#pragma once

#include "cbase.h"
#include "zed_net.h"
#include "mom_player.h"
#include "mom_shareddefs.h"
#include "mom_online_ghost.h"

struct MyThreadParams_t{}; //empty class so we can force the threaded function to work xd

class CMomentumGhostClient : public CAutoGameSystemPerFrame
{
public:
    CMomentumGhostClient(const char *pName) : CAutoGameSystemPerFrame(pName), m_sLobbyID(), m_sHostID(), m_bHostingLobby(false), m_flNextUpdateTime(-1.0f)
    {
        SetDefLessFunc(m_mapOnlineGhosts);
    }

    //bool Init() OVERRIDE; MOM_TODO: Set state variables here?
    void PostInit() OVERRIDE;
    //void LevelInitPreEntity() OVERRIDE;
    void LevelInitPostEntity() OVERRIDE;
    //void LevelShutdownPreEntity() OVERRIDE;
    void LevelShutdownPreEntity() OVERRIDE;
    void FrameUpdatePreEntityThink() OVERRIDE;
    void Shutdown() OVERRIDE;

    void HandleLobbyCreated(LobbyCreated_t *pCreated, bool IOFailure);

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
    void GetLobbyMemberSteamData(CSteamID pMember);

    static bool initGhostClient();
    static bool exitGhostClient();
    static bool connectToGhostServer(const char* host, unsigned short port);
    static unsigned sendAndRecieveData(void *params);
    static bool isGhostClientConnected() { return m_ghostClientConnected && (m_socket.ready == 0); }
    static bool SendSignonMessage();
    static ghostNetFrame_t CreateNewNetFrame(CMomentumPlayer *pPlayer);
    static ghostAppearance_t CreateAppearance(CMomentumPlayer* pPlayer) { return pPlayer->m_playerAppearanceProps; }
    static bool SendAppearanceData(ghostAppearance_t apps);
    static bool SendNetFrame(ghostNetFrame_t frame);
private:
    static zed_net_socket_t m_socket;
    static zed_net_address_t m_address;
    static bool m_ghostClientConnected, m_bRanThread;
    static char data[256];

    static CThreadMutex m_mtxGhostPlayers, m_mtxpPlayer;
    static CUtlVector<CMomentumOnlineGhostEntity*> ghostPlayers;
    static CMomentumPlayer *m_pPlayer;
    static uint64 m_SteamID;
    static ghostAppearance_t oldAppearance;

    static CUtlMap<uint64, CMomentumOnlineGhostEntity*> m_mapOnlineGhosts;

    void CheckToAdd(CSteamID *pSteamID);

    CSteamID m_sLobbyID;

    CSteamID m_sHostID;

    bool m_bHostingLobby;
    float m_flNextUpdateTime;

    CCallResult<CMomentumGhostClient, LobbyCreated_t> m_cLobbyCreated;
};

extern CMomentumGhostClient *g_pMomentumGhostClient;
