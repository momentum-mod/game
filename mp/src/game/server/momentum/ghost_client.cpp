#include "cbase.h"
#include "ghost_client.h"
#include "mom_online_ghost.h"
#include "icommandline.h"
#include "mom_lobby_system.h"

#include "tier0/memdbgon.h"

ConVar mm_updaterate("mom_ghost_online_updaterate", "25",
    FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE,
    "Number of updates per second for online ghosts.\n", true, 1.0f, true, 50.0f);

CON_COMMAND(mom_spectate, "Start spectating if there are ghosts currently being played.")
{
    auto pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer && !pPlayer->IsObserver())
    {
        auto pNext = pPlayer->FindNextObserverTarget(false);
        if (pNext)
        {
            // Setting ob target first is needed for the specGUI panel to update properly
            pPlayer->SetObserverTarget(pNext);
            pPlayer->StartObserverMode(OBS_MODE_IN_EYE);
        }
    }
}

CON_COMMAND(mom_spectate_stop, "Stop spectating.")
{
    auto pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer)
    {
        pPlayer->StopSpectating();
        g_pMomentumTimer->DispatchTimerStateMessage(pPlayer, false);
        // We're piggybacking on this event because it's basically the same as the X on the mapfinished panel
        IGameEvent *pClosePanel = gameeventmanager->CreateEvent("mapfinished_panel_closed");
        if (pClosePanel)
        {
            // Fire this event so other classes can get at this
            gameeventmanager->FireEvent(pClosePanel);
        }
    }
}
CMomentumPlayer* CMomentumGhostClient::m_pPlayer = nullptr;
CUtlMap<uint64, CMomentumOnlineGhostEntity*> CMomentumGhostClient::m_mapOnlineGhosts;
CMomentumGhostClient *CMomentumGhostClient::m_pInstance = nullptr;

CMomentumGhostClient::CMomentumGhostClient(const char* pName) : CAutoGameSystemPerFrame(pName)
{
    SetDefLessFunc(m_mapOnlineGhosts);
    m_pInstance = this;
}

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
    if (!FStrEq(gpGlobals->mapname.ToCStr(), "")) // Don't send our shutdown message from the menu
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

void CMomentumGhostClient::ResetOtherAppearanceData()
{
    // MOM_TODO: g_pMomentumServerSystem->ResetOtherAppearanceData();
    g_pMomentumLobbySystem->ResetOtherAppearanceData();
}

void CMomentumGhostClient::SendAppearanceData(ghostAppearance_t appearance)
{
    // MOM_TODO: g_pMomentumServerSystem->SetAppearance(appearance);
    g_pMomentumLobbySystem->SetAppearanceInMemberData(appearance);
}

void CMomentumGhostClient::SetSpectatorTarget(CSteamID target, bool bStartedSpectating)
{
    // MOM_TODO: g_pMomentumServerSystem->SetSpectatorTarget(target, bStartedSpectating)
    g_pMomentumLobbySystem->SetSpectatorTarget(target, bStartedSpectating);
}

void CMomentumGhostClient::SendDecalPacket(DecalPacket_t *packet)
{
    // MOM_TODO: g_pMomentumServerSystem->SendDecalPacket(packet);
    g_pMomentumLobbySystem->SendDecalPacket(packet);
}

bool CMomentumGhostClient::CreateNewNetFrame(PositionPacket_t &into)
{
    if (m_pPlayer && !m_pPlayer->IsSpectatingGhost())
    {
        into = PositionPacket_t(
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