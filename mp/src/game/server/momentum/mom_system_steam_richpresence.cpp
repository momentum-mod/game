#include "cbase.h"

#include "mom_system_steam_richpresence.h"

#include "fmtstr.h"
#include "ghost_client.h"
#include "mom_lobby_system.h"
#include "mom_shareddefs.h"
#include "mom_system_gamemode.h"

#include "tier0/memdbgon.h"

static void SteamRichPresenceVarUpdate(IConVar *pVar, const char *pOldVal, float fOldValue)
{
    const ConVarRef var(pVar);
    if (var.GetBool())
    {
        g_pSteamRichPresence->Update();
    }
    else
    {
        CHECK_STEAM_API(SteamFriends());
        SteamFriends()->ClearRichPresence();
    }
}

static MAKE_TOGGLE_CONVAR_C(mom_steam_rich_presence, "1", FCVAR_ARCHIVE, "Toggles steam rich presence, 0 = OFF, 1 = ON\n", SteamRichPresenceVarUpdate);

CSteamRichPresenceSystem::CSteamRichPresenceSystem() : CAutoGameSystem("SteamRichPresenceSystem")
{
    
}

void CSteamRichPresenceSystem::Update(bool bLevelShutdown /* = false*/)
{
    CHECK_STEAM_API(SteamFriends());
    CHECK_STEAM_API(SteamMatchmaking());

    if (!mom_steam_rich_presence.GetBool())
        return;

    const auto lobbyID = CMomentumLobbySystem::m_sLobbyID;
    const auto bInLobby = g_pMomentumGhostClient->IsInOnlineSession();
    const auto pMap = bLevelShutdown ? nullptr : STRING(gpGlobals->mapname);
    const auto numPlayers = SteamMatchmaking()->GetNumLobbyMembers(lobbyID);
    
    const auto pConnectStr = bInLobby ? CFmtStr("+connect_lobby %llu", lobbyID.ConvertToUint64()).Get() : nullptr;
    SteamFriends()->SetRichPresence("connect", pConnectStr);

    // New Steam Rich Presence keys
    SteamFriends()->SetRichPresence("steam_player_group", bInLobby ? CFmtStr("%llu", lobbyID.ConvertToUint64()).Get() : nullptr);
    SteamFriends()->SetRichPresence("steam_player_group_size", bInLobby ? CFmtStr("%i", numPlayers).Get() : nullptr);

    const char *pSteamStatusStr;
    if (!pMap || !pMap[0] || FStrEq(pMap, "credits") || gpGlobals->eLoadType == MapLoad_Background)
    {
        pSteamStatusStr = "#Main_Menu";

        SteamFriends()->SetRichPresence("map", nullptr);
        SteamFriends()->SetRichPresence("gamemode", nullptr);

        SteamFriends()->SetRichPresence("status", nullptr);
    }
    else
    {
        pSteamStatusStr = "#Playing_Game";

        SteamFriends()->SetRichPresence("map", pMap);
        SteamFriends()->SetRichPresence("gamemode", g_szGameModes[g_pGameModeSystem->GetGameMode()->GetType()]);

        // Status unfortunately doesn't have this nice system in place (uses old, single-string one)
        char gameInfoStr[64];
        const auto pFormatStr = numPlayers <= 1 ? "%s on %s" : "%s on %s with %i other player%s";
        V_snprintf(gameInfoStr, 64, pFormatStr,
            g_pGameModeSystem->GetGameMode()->GetStatusString(),
            pMap,
            numPlayers - 1,
            numPlayers > 2 ? "s" : ""); // It's "other" (than the local player) players, so > 2 is right!

        SteamFriends()->SetRichPresence("status", gameInfoStr);
    }

    SteamFriends()->SetRichPresence("steam_display", pSteamStatusStr);
}

static CSteamRichPresenceSystem s_SteamRichPres;
CSteamRichPresenceSystem *g_pSteamRichPresence = &s_SteamRichPres;