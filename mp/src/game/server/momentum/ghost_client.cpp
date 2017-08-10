#include "cbase.h"
#include "ghost_client.h"
#include "util/mom_util.h"
#include "mom_online_ghost.h"
#include "icommandline.h"

#include "tier0/memdbgon.h"

ConVar mm_updaterate("mom_ghost_online_updaterate", "25",
    FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE,
    "Number of updates per second to and from the ghost server.\n", true, 1.0f, true, 1000.0f);

ConVar mm_timeOutDuration("mom_ghost_online_timeout_duration", "10",
    FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE,
    "Seconds to wait when timimg out from a ghost server.\n", true, 5.0f, true, 30.0f);

//we have to wait a few ticks to let the interpolation catch up with our ghosts!
ConVar mm_lerpRatio("mom_ghost_online_lerp_ratio", "2",
    FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE,
    "Number of ticks to wait before updating ghosts, to allow client to interpolate.\n", true, 0.0f, true, 10.0f);


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

    g_pMomentumLobbySystem->LevelChange(gpGlobals->mapname.ToCStr());
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
ghostNetFrame_t CMomentumGhostClient::CreateNewNetFrame(CMomentumPlayer *pPlayer)
{
    Assert(pPlayer);
    return ghostNetFrame_t(
        pPlayer->EyeAngles(),
        pPlayer->GetAbsOrigin(),
        pPlayer->GetAbsVelocity(),
        pPlayer->GetViewOffset().z,
        pPlayer->m_nButtons);
}

static CMomentumGhostClient s_MOMGhostClient("CMomentumGhostClient");
CMomentumGhostClient *g_pMomentumGhostClient = &s_MOMGhostClient;