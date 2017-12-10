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
        CBaseEntity *pTarget = nullptr;
        // If they specified a target, let's go to them
        if (args.ArgC() > 1)
        {
            uint64 target = Q_atoui64(args.Arg(1));
            pTarget = g_pMomentumGhostClient->GetOnlineGhostEntityFromID(target);
        }

        if (!pTarget)
            pTarget = pPlayer->FindNextObserverTarget(false);

        // One last null check
        if (pTarget)
        {
            // Setting ob target first is needed for the specGUI panel to update properly
            pPlayer->SetObserverTarget(pTarget);
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
CMomentumGhostClient *CMomentumGhostClient::m_pInstance = nullptr;

CMomentumGhostClient::CMomentumGhostClient(const char* pName) : CAutoGameSystemPerFrame(pName)
{
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
    if (!FStrEq(gpGlobals->mapname.ToCStr(), "")) // Don't send our shutdown message from the menu
        g_pMomentumLobbySystem->LevelChange(nullptr);
}

void CMomentumGhostClient::LevelShutdownPreEntity()
{
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
    // MOM_TODO: g_pMomentumServerSystem->ClearCurrentGhosts(bRemoveGhostEnts)
    g_pMomentumLobbySystem->ClearCurrentGhosts(bRemoveGhostEnts);
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

CMomentumOnlineGhostEntity* CMomentumGhostClient::GetOnlineGhostEntityFromID(const uint64& id)
{
    // MOM_TODO: Obviously determine if we're in a lobby or server here
    // MOM_TODO: g_pMomentumServerSystem->GetServerMemberEntity(id);
    return g_pMomentumLobbySystem->GetLobbyMemberEntity(id);
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