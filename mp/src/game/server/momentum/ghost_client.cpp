#include "cbase.h"
#include "ghost_client.h"
#include "mom_online_ghost.h"
#include "icommandline.h"
#include "mom_lobby_system.h"
#include "mom_player_shared.h"
#include "mom_timer.h"

#include "tier0/memdbgon.h"

ConVar mm_updaterate("mom_ghost_online_updaterate", "25",
    FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE,
    "Number of updates per second for online ghosts.\n", true, 1.0f, true, 50.0f);

CON_COMMAND(mom_spectate, "Start spectating if there are ghosts currently being played.")
{
    if (gpGlobals->eLoadType == MapLoad_Background || FStrEq(gpGlobals->mapname.ToCStr(), "credits"))
        return;

    auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (pPlayer && !pPlayer->IsObserver())
    {
        CBaseEntity *pTarget = nullptr;
        // If they specified a target, let's go to them
        if (args.ArgC() > 1)
        {
            uint64 target = Q_atoui64(args.Arg(1));
            const auto pGhost = g_pMomentumGhostClient->GetOnlineGhostEntityFromID(target);
            if (pGhost && !pGhost->m_bSpectating.Get())
                pTarget = pGhost;
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
        else
        {
            // Not valid but they still want to spectate? Let's go in roaming mode
            pPlayer->StartObserverMode(OBS_MODE_ROAMING);
        }
    }
}

CON_COMMAND(mom_spectate_stop, "Stop spectating.")
{
    auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (pPlayer)
    {
        pPlayer->StopSpectating();
        // We're piggybacking on this event because it's basically the same as the X on the mapfinished panel
        IGameEvent *pClosePanel = gameeventmanager->CreateEvent("mapfinished_panel_closed");
        if (pClosePanel)
        {
            // Fire this event so other classes can get at this
            gameeventmanager->FireEvent(pClosePanel);
        }
    }
}

CMomentumGhostClient::CMomentumGhostClient(const char* pName) : CAutoGameSystemPerFrame(pName)
{
}

void CMomentumGhostClient::PostInit()
{
    //Log("================= COMMAND LINE: %s\n", CommandLine()->GetCmdLine());
    const char *pLobbyID = CommandLine()->ParmValue("+connect_lobby", nullptr);
    g_pMomentumLobbySystem->TryJoinLobbyFromString(pLobbyID);
}

void CMomentumGhostClient::LevelInitPostEntity()
{
    // MOM_TODO: AdvertiseGame needs to use k_steamIDNonSteamGS and pass the IP (as hex) and port if it is inside a server 
    // SteamUser()->AdvertiseGame(SteamUser()->GetSteamID(), 0, 0); // Gives game info of current server, useful if actually on server
    // SteamFriends()->SetRichPresence("connect", "blah"); // Allows them to click "Join game" from Steam
    g_pMomentumLobbySystem->LevelChange(gpGlobals->mapname.ToCStr());
}

void CMomentumGhostClient::LevelShutdownPostEntity()
{
    if (!FStrEq(gpGlobals->mapname.ToCStr(), "")) // Don't send our shutdown message from the menu
        g_pMomentumLobbySystem->LevelChange(nullptr);
}

void CMomentumGhostClient::LevelShutdownPreEntity()
{
}

void CMomentumGhostClient::FrameUpdatePreEntityThink()
{
    g_pMomentumLobbySystem->SendAndReceiveP2PPackets();
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

void CMomentumGhostClient::SendAppearanceData(GhostAppearance_t appearance)
{
    // MOM_TODO: g_pMomentumServerSystem->SetAppearance(appearance);
    g_pMomentumLobbySystem->SetAppearanceInMemberData(appearance);
}

void CMomentumGhostClient::SetIsSpectating(bool state)
{
    // MOM_TODO g_pMomentumServerSystem->SetIsSpectating(state);
    g_pMomentumLobbySystem->SetIsSpectating(state);
}

void CMomentumGhostClient::SetSpectatorTarget(CSteamID target, bool bStartedSpectating, bool bLeft)
{
    // MOM_TODO: g_pMomentumServerSystem->SetSpectatorTarget(target, bStartedSpectating)
    g_pMomentumLobbySystem->SetSpectatorTarget(target, bStartedSpectating, bLeft);
}

void CMomentumGhostClient::SendDecalPacket(DecalPacket *packet)
{
    static ConVarRef host_timescale("host_timescale");

    if (CloseEnough(host_timescale.GetFloat(), 1.0f, FLT_EPSILON))
    {
        // MOM_TODO: g_pMomentumServerSystem->SendDecalPacket(packet);
        g_pMomentumLobbySystem->SendDecalPacket(packet);
    }
    // MOM_TODO: else let the player know their decal packets aren't being sent?
}

bool CMomentumGhostClient::SendSavelocReqPacket(CSteamID& target, SavelocReqPacket* packet)
{
    // MOM_TODO: g_pMomentumServerSystem->SendSavelocReqPacket(target, packet);
    return g_pMomentumLobbySystem->SendSavelocReqPacket(target, packet);
}

bool CMomentumGhostClient::IsInOnlineSession()
{
    return g_pMomentumLobbySystem->LobbyValid(); /*MOM_TODO: || g_pMomentumServerSystem->ServerValid();*/
}

CMomentumOnlineGhostEntity* CMomentumGhostClient::GetOnlineGhostEntityFromID(const uint64& id)
{
    // MOM_TODO: Obviously determine if we're in a lobby or server here
    // MOM_TODO: g_pMomentumServerSystem->GetServerMemberEntity(id);
    return g_pMomentumLobbySystem->GetLobbyMemberEntity(id);
}

CUtlMap<uint64, CMomentumOnlineGhostEntity*> *CMomentumGhostClient::GetOnlineGhostMap()
{
    // MOM_TODO: if (g_pMomentumServerSystem->IsConnected()) // or something
    //            g_pMomentumServerSystem->GetOnlineEntMap();
    // else
    return g_pMomentumLobbySystem->GetOnlineEntMap();
}

bool CMomentumGhostClient::CreateNewNetFrame(PositionPacket &into)
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (pPlayer && !pPlayer->IsSpectatingGhost())
    {
        const Vector &orig = pPlayer->GetAbsOrigin(), &vel = pPlayer->GetAbsVelocity();
        const QAngle &eye = pPlayer->EyeAngles();

        if (!(orig.IsValid() && vel.IsValid() && eye.IsValid()))
            return false;

        into = PositionPacket(
            eye,
            orig,
            vel,
            pPlayer->GetViewOffset().z,
            pPlayer->m_nButtons);

        return true;
    }

    return false;
}

static CMomentumGhostClient s_MOMGhostClient("CMomentumGhostClient");
CMomentumGhostClient *g_pMomentumGhostClient = &s_MOMGhostClient;