#include "cbase.h"
#include "ghost_client.h"
#include "util/mom_util.h"
#include "mom_online_ghost.h"
#include "icommandline.h"

#include "tier0/memdbgon.h"

ConVar mm_updaterate("mom_ghost_online_updaterate", "25",
    FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE,
    "Number of updates per second for online ghosts.\n", true, 1.0f, true, 50.0f);

CMomentumPlayer* CMomentumGhostClient::m_pPlayer = nullptr;
CUtlMap<uint64, CMomentumOnlineGhostEntity*> CMomentumGhostClient::m_mapOnlineGhosts;
CMomentumGhostClient *CMomentumGhostClient::m_pInstance = nullptr;

void CMomentumGhostClient::PostInit()
{
    //Log("================= COMMAND LINE: %s\n", CommandLine()->GetCmdLine());
    const char *pLobbyID = CommandLine()->ParmValue("+connect_lobby", nullptr);
    g_pMomentumLobbySystem->JoinLobbyFromString(pLobbyID);
}

void CMomentumGhostClient::LevelInitPostEntity()
{
    // MOM_TODO: AdvertiseGame needs to use k_steamIDNonSteamGS and pass the IP (as hex) and port if it is inside a server 
    // steamapicontext->SteamUser()->AdvertiseGame(steamapicontext->SteamUser()->GetSteamID(), 0, 0); // Gives game info of current server, useful if actually on server
    // steamapicontext->SteamFriends()->SetRichPresence("connect", "blah"); // Allows them to click "Join game" from Steam

    m_pPlayer = ToCMOMPlayer(UTIL_GetListenServerHost());
    g_pMomentumLobbySystem->LevelChange(gpGlobals->mapname.ToCStr());
}

void CMomentumGhostClient::LevelShutdownPostEntity()
{
    m_pPlayer = nullptr;
}

void CMomentumGhostClient::LevelShutdownPreEntity()
{
    g_pMomentumLobbySystem->LevelChange(nullptr);
}

void CMomentumGhostClient::FrameUpdatePreEntityThink()
{
    m_pPlayer = ToCMOMPlayer(UTIL_GetListenServerHost());
    g_pMomentumLobbySystem->SendAndRecieveP2PPackets();
}

void CMomentumGhostClient::Shutdown()
{
    g_pMomentumLobbySystem->LeaveLobby(); // Leave the lobby if we're still in it
}

void CMomentumGhostClient::ClearCurrentGhosts(bool bRemoveGhostEnts)
{
    // We have to remove every entity manually if we left this lobby
    if (m_mapOnlineGhosts.Count() > 0)
    {
        if (bRemoveGhostEnts)
        {
            unsigned short currIndx = m_mapOnlineGhosts.FirstInorder();
            while (currIndx != m_mapOnlineGhosts.InvalidIndex())
            {
                CMomentumOnlineGhostEntity *pEnt = m_mapOnlineGhosts[currIndx];
                if (pEnt)
                    pEnt->Remove();

                currIndx = m_mapOnlineGhosts.NextInorder(currIndx);
            }
        }
        
        m_mapOnlineGhosts.RemoveAll(); // No need to purge, the game handles the entities' memory
    }
}

void CMomentumGhostClient::SendChatMessage(char* pMessage)
{
    // MOM_TODO: g_pMomentumServerSystem->SendChatMessage(pMessage)
    g_pMomentumLobbySystem->SendChatMessage(pMessage);
}

bool CMomentumGhostClient::CreateNewNetFrame(ghostNetFrame_t &into)
{
    if (m_pPlayer && !m_pPlayer->IsSpectatingGhost())
    {
        into = ghostNetFrame_t(
            m_pPlayer->EyeAngles(),
            m_pPlayer->GetAbsOrigin(),
            m_pPlayer->GetAbsVelocity(),
            m_pPlayer->GetViewOffset().z,
            m_pPlayer->m_nButtons);

        return true;
    }

    return false;
}

static CMomentumGhostClient s_MOMGhostClient("CMomentumGhostClient");
CMomentumGhostClient *g_pMomentumGhostClient = &s_MOMGhostClient;