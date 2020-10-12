#include "cbase.h"

#include "mom_system_discord.h"
#include "mom_system_gamemode.h"

#include "discord_rpc.h"

#include <time.h>
#include "fmtstr.h"
#include "mom_player_shared.h"
#include "mom_shareddefs.h"
#include "util/mom_util.h"
#include "steam/steam_api.h"

#include "tier0/memdbgon.h"

#define DISCORD_APP_ID "378351756104564738"
#define MOM_STEAM_ID "669270"

#define MAIN_MENU_STR "In Menus"
#define MOM_ICON_LOGO "mom"

// How many frames to wait before updating discord
// (some things are still updated each frame such as checking callbacks)
#define DISCORD_FRAME_UPDATE_FREQ 600

static void ToggleDiscordRPCState(IConVar *pVar, const char *pOldVal, float)
{
    const ConVarRef ref(pVar);
    if (ref.GetBool())
    {
        g_pMomentumDiscord->DiscordInit();
        g_pMomentumDiscord->DiscordUpdate();
    }
    else
    {
        Discord_Shutdown();
    }
}
static MAKE_TOGGLE_CONVAR_C(mom_discord_enable, "1", FCVAR_ARCHIVE, "Toggles the Discord RPC functionality. 0 = OFF, 1 = ON.\n", ToggleDiscordRPCState);

#ifdef DEBUG
CON_COMMAND(mom_discord_debug, "Help debug discord integration\n")
{
    DevLog("Party ID: %s\n", g_pMomentumDiscord->m_szDiscordPartyId);
    DevLog("Spectate: %s\n", g_pMomentumDiscord->m_szDiscordSpectateSecret);
    DevLog("Join:     %s\n", g_pMomentumDiscord->m_szDiscordJoinSecret);
}
#endif

CMomentumDiscord::CMomentumDiscord() : CAutoGameSystemPerFrame("DiscordRPC")
{
    m_ulSpectateTargetUser = 0;
    m_szSpectateTargetUser[0] = '\0';
    m_kJoinSpectateState = Null;
    m_iDiscordStartTimestamp = 0;
    m_iDiscordEndTimestamp = 0;
    m_iDiscordPartySize = 0;
    m_iDiscordPartyMax = 0;
    m_iDiscordInstance = 0;
    m_iUpdateFrame = 0;
    m_bInMap = false;
    m_bValid = false;

    m_szSpectateTargetLobby[0] = '\0';

    // Discord fields
    m_szDiscordState[0] = '\0';
    m_szDiscordDetails[0] = '\0';
    m_szDiscordLargeImageKey[0] = '\0';
    m_szDiscordLargeImageText[0] = '\0';
    m_szDiscordSmallImageKey[0] = '\0';
    m_szDiscordSmallImageText[0] = '\0';
    m_szDiscordPartyId[0] = '\0';
    m_szDiscordMatchSecret[0] = '\0';
    m_szDiscordSpectateSecret[0] = '\0';
    m_szDiscordJoinSecret[0] = '\0';
    m_sSteamLobbyID = CSteamID();
    m_sSteamUserID = CSteamID();
}

// ---------------------------- CAutoGameSystem ----------------------------- //

// Called once on game start
void CMomentumDiscord::PostInit()
{
    if (!(SteamUser() && SteamMatchmaking()))
    {
        Warning("Couldn't load required Steam APIs for Discord RPC!\n");
        m_bValid = false;
    }
    else
    {
        ListenForGameEvent("lobby_leave");
        ListenForGameEvent("zone_enter");
        ListenForGameEvent("zone_exit");
        ListenForGameEvent("spec_target_updated");
        ListenForGameEvent("spec_start");
        ListenForGameEvent("spec_stop");

        DiscordInit();

        // Default discord information
        V_strncpy(m_szDiscordState, MAIN_MENU_STR, sizeof(m_szDiscordState));
        V_strncpy(m_szDiscordLargeImageKey, MOM_ICON_LOGO, sizeof(m_szDiscordLargeImageKey));

        m_sSteamUserID = SteamUser()->GetSteamID();
        m_bValid = true;
    }
}

// Called each time a map is joined after entities are initialized
void CMomentumDiscord::LevelInitPostEntity()
{
    if (!m_bValid)
        return;

    // Ignore background map(s)
    if (!Q_strncmp(MapName(), "bg_", 3))
    {
        V_strncpy(m_szDiscordState, MAIN_MENU_STR, sizeof(m_szDiscordState));
        V_strncpy(m_szDiscordLargeImageKey, MOM_ICON_LOGO, sizeof(m_szDiscordLargeImageKey));
        return;
    }

    m_bInMap = true;

    V_strncpy(m_szDiscordLargeImageKey, g_pGameModeSystem->GetGameMode()->GetDiscordIcon(), sizeof(m_szDiscordLargeImageKey));
    if (g_pGameModeSystem->GameModeIs(GAMEMODE_UNKNOWN))
    {
        m_szDiscordSmallImageKey[0] = '\0';
    }
    else
    {
        V_strncpy(m_szDiscordSmallImageKey, MOM_ICON_LOGO, sizeof(m_szDiscordSmallImageKey));
    }

    V_strncpy(m_szDiscordState, g_pGameModeSystem->GetGameMode()->GetStatusString(), sizeof(m_szDiscordState));
    V_strncpy(m_szDiscordDetails, MapName(), sizeof(m_szDiscordDetails));
    V_strncpy(m_szDiscordLargeImageText, g_pGameModeSystem->GetGameMode()->GetStatusString(), sizeof(m_szDiscordLargeImageText));
    m_iDiscordStartTimestamp = time(nullptr);

    // Check to see if we are joining this map to spectate a player
    if (m_kJoinSpectateState == WaitOnMap)
    {
        m_kJoinSpectateState = WaitOnNone;
        SpectateTargetFromDiscord();
    }
}

// Called each time a map is left
void CMomentumDiscord::LevelShutdownPreEntity()
{
    if (!m_bValid)
        return;

    m_bInMap = false;
    ClearDiscordFields(false); // Pass false to retain party/lobby related fields
    V_strncpy(m_szDiscordState, MAIN_MENU_STR, sizeof(m_szDiscordState));
    V_strncpy(m_szDiscordLargeImageKey, MOM_ICON_LOGO, sizeof(m_szDiscordLargeImageKey));
}

// Called every frame
void CMomentumDiscord::Update(float frametime)
{
    if (!(m_bValid && mom_discord_enable.GetBool()))
        return;

    if (m_iUpdateFrame++ == DISCORD_FRAME_UPDATE_FREQ)
    {
        // Stuff that can be done every ~1 second
        m_iUpdateFrame = 0;
        DiscordUpdate();
    }

// Do the rest of this stuff every frame
#ifdef DISCORD_DISABLE_IO_THREAD
    Discord_UpdateConnection();
#endif
    Discord_RunCallbacks();
}

// Called on game quit
void CMomentumDiscord::Shutdown()
{
    if (m_bValid && mom_discord_enable.GetBool())
        Discord_Shutdown();
}

// -------------------------- Steam API Callbacks --------------------------- //

// When a user joins a lobby
void CMomentumDiscord::HandleLobbyEnter(LobbyEnter_t *pParam)
{
    if (pParam->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseSuccess)
    {
        m_sSteamLobbyID = CSteamID(pParam->m_ulSteamIDLobby);
        UpdateDiscordPartyIdFromSteam();
        UpdateLobbyNumbers();
    }
}

void CMomentumDiscord::HandleLobbyDataUpdate(LobbyDataUpdate_t *pParam)
{
    if (CSteamID(pParam->m_ulSteamIDLobby) != m_sSteamLobbyID)
    {
        m_sSteamLobbyID = CSteamID(pParam->m_ulSteamIDLobby);
        UpdateDiscordPartyIdFromSteam();
    }

    OnSteamLobbyUpdate();
}

void CMomentumDiscord::HandleLobbyChatUpdate(LobbyChatUpdate_t *pParam)
{
    if (pParam->m_ulSteamIDLobby != m_sSteamLobbyID.ConvertToUint64())
    {
        m_sSteamLobbyID = CSteamID(pParam->m_ulSteamIDLobby);
        UpdateDiscordPartyIdFromSteam();
    }

    OnSteamLobbyUpdate();
}

// --------------------------- CGameEventListner ---------------------------- //

// Called when there is a game event we are listening for
// Subscribe to events in `PostInit`
void CMomentumDiscord::FireGameEvent(IGameEvent *event)
{
    const char *pName = event->GetName();
    const bool bZoneEnter = FStrEq(pName, "zone_enter");
    const bool bZoneExit = FStrEq(pName, "zone_exit");
    if (FStrEq(pName, "lobby_leave"))
    {
        m_sSteamLobbyID.Clear();
        m_iDiscordPartySize = 0;
        m_iDiscordPartyMax = 0;
        m_szDiscordPartyId[0] = '\0';
        m_szDiscordJoinSecret[0] = '\0';
        m_szDiscordSpectateSecret[0] = '\0';
    }
    else if (bZoneEnter || bZoneExit)
    {
        const int entIndx = event->GetInt("ent");
        if (entIndx == engine->GetLocalPlayer())
        {
            const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
            const int currentZone = event->GetInt("num", -1);
            if (bZoneEnter && currentZone == 1)
            {
                Q_strncpy(m_szDiscordState, "Start Zone", sizeof(m_szDiscordState));
            }
            else if (bZoneEnter && currentZone == 0)
            {
                Q_strncpy(m_szDiscordState, "End Zone", sizeof(m_szDiscordState));
            }
            else if (pPlayer && pPlayer->m_iZoneCount[pPlayer->m_Data.m_iCurrentTrack] > 0)
            {
                const bool linearTrack = pPlayer->m_iLinearTracks[pPlayer->m_Data.m_iCurrentTrack];

                Q_snprintf(m_szDiscordState, sizeof(m_szDiscordState), "%s %i/%i", linearTrack ? "Checkpoint" : "Stage",
                           currentZone, // Current stage/checkpoint
                           pPlayer->m_iZoneCount[pPlayer->m_Data.m_iCurrentTrack]
                );
            }
            else
            {
                Q_strncpy(m_szDiscordState, g_pGameModeSystem->GetGameMode()->GetStatusString(), sizeof(m_szDiscordState));
            }
        }
    }
    else if (FStrEq(pName, "spec_start") || FStrEq(pName, "spec_target_updated"))
    {
        const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
        CMomRunEntity *pSpecTarget = pPlayer->GetCurrentUIEntity();
        if (pSpecTarget->GetEntType() == RUN_ENT_REPLAY)
        {
            Q_strncpy(m_szDiscordState, "Watching replay", sizeof(m_szDiscordState));
        }
        else if (pPlayer->IsObserver())
        {
            Q_strncpy(m_szDiscordState, "Spectating", sizeof(m_szDiscordState));
        }
    }
    else if (FStrEq(pName, "spec_stop"))
    {
        Q_strncpy(m_szDiscordState, g_pGameModeSystem->GetGameMode()->GetStatusString(), sizeof(m_szDiscordState));
    }

    DiscordUpdate();
}

// ----------------------------- Custom Methods ----------------------------- //

// Get the current map of a player from their steam ID
const char *CMomentumDiscord::GetMapOfPlayerFromSteamID(CSteamID *steamID)
{
    if (!SteamMatchmaking() || !m_sSteamLobbyID.IsValid())
    {
        return "";
    }
    else
    {
        return SteamMatchmaking()->GetLobbyMemberData(m_sSteamLobbyID, *steamID, LOBBY_DATA_MAP);
    }
}

// Joins a lobby from the Steam Lobby ID
// returns true if we had to change the lobby
bool CMomentumDiscord::JoinSteamLobbyFromID(const char *lobbyID)
{
    if (m_sSteamLobbyID.IsValid() &&
        FStrEq(lobbyID, CFmtStr("%lld", m_sSteamLobbyID.ConvertToUint64())))
    {
        // If we are already in the lobby, don't join
        return false;
    }
    else if (m_sSteamLobbyID.IsValid())
    {
        // If we are in a different lobby we need to leave
        MomUtil::DispatchConCommand("mom_lobby_leave");
    }

    // Finally join the new lobby
    MomUtil::DispatchConCommand(CFmtStr("connect_lobby %s", lobbyID));

    return true;
}

// Joins the same map as the player referenced to by `steamID`
// returns true if we had to change maps or couldn't get the map
bool CMomentumDiscord::JoinMapFromUserSteamID(uint64 steamID)
{
    CSteamID targetPlayerSteamID = CSteamID(steamID);
    const char *targetMapName = GetMapOfPlayerFromSteamID(&targetPlayerSteamID);

    if (!(targetMapName && targetMapName[0]))
    {
        return false;
    }

    if (MapName() && !V_strcmp(targetMapName, MapName()))
    {
        // Already on the same map
        return false;
    }
    else
    {
        MomUtil::DispatchConCommand(CFmtStr("map %s", targetMapName));
    }

    return true;
}

// When there is an update from the steam lobby API
void CMomentumDiscord::OnSteamLobbyUpdate()
{
    // Check to see if we are waiting to spectate a player
    if (m_kJoinSpectateState == WaitOnLobbyAndMap)
    {
        m_kJoinSpectateState = WaitOnMap;
        SpectateTargetFromDiscord();
    }
    else if (m_kJoinSpectateState == WaitOnLobby)
    {
        m_kJoinSpectateState = WaitOnNone;
        SpectateTargetFromDiscord();
    }

    UpdateLobbyNumbers();
}

// Handle spectating a player (spectating is more complicated than it would seem)
// This method acts as a state machine as we may need to wait for things to happen
// such as joining a steam lobby, and/or joining a map, before we can spectate
void CMomentumDiscord::SpectateTargetFromDiscord()
{
    switch (m_kJoinSpectateState)
    {
    case WaitOnLobbyAndMap:
        if (!JoinSteamLobbyFromID(m_szSpectateTargetLobby))
        {
            m_kJoinSpectateState = WaitOnMap;
            SpectateTargetFromDiscord();
        }
        break;

    case WaitOnLobby:
        if (!JoinSteamLobbyFromID(m_szSpectateTargetLobby))
        {
            m_kJoinSpectateState = WaitOnNone;
            SpectateTargetFromDiscord();
        }
        break;

    case WaitOnMap:
        if (!JoinMapFromUserSteamID(m_ulSpectateTargetUser))
        {
            m_kJoinSpectateState = WaitOnNone;
            SpectateTargetFromDiscord();
        }
        break;

    case WaitOnNone:
        m_kJoinSpectateState = Null;
        SpecPlayerFromSteamId(m_szSpectateTargetUser);
        break;

    case Null:
    default:
        return;
    }
}

// Causes the current player to start spectating the target user
void CMomentumDiscord::SpecPlayerFromSteamId(const char *steamID)
{
    DevLog("CMomentumDiscord::SpecPlayerFromSteamId(%s)\n", steamID);

    // MOM_TODO: We probably should check if the target player is already spectating someone, then set the spectate
    // target them instead Or we could adjust the `mom_spectate` command to handle this automatically
    MomUtil::DispatchConCommand(CFmtStr("mom_spectate %s", steamID));
}

// Updates the current and max party size based upon the current lobby
void CMomentumDiscord::UpdateLobbyNumbers()
{
    if (m_sSteamLobbyID.IsValid())
    {
        m_iDiscordPartySize = SteamMatchmaking()->GetNumLobbyMembers(m_sSteamLobbyID);
        m_iDiscordPartyMax = SteamMatchmaking()->GetLobbyMemberLimit(m_sSteamLobbyID);
    }
}

// Keeps the discord party ID and secrets up to date from the steam data
void CMomentumDiscord::UpdateDiscordPartyIdFromSteam()
{
    if (m_sSteamLobbyID.IsValid())
    {
        const auto lobbyID = m_sSteamLobbyID.ConvertToUint64();

        // The party ID is just the Steam Lobby ID
        V_snprintf(m_szDiscordPartyId, sizeof(m_szDiscordPartyId), "%lld", lobbyID);

        // We could do something to further "encrypt" the secrets but it's probably fine
        if (m_sSteamUserID.IsValid())
        {
            // If we found a valid steam user ID add it into the secret
            V_snprintf(m_szDiscordJoinSecret, sizeof(m_szDiscordJoinSecret), "J%lld;%lld", lobbyID, m_sSteamUserID.ConvertToUint64());
        }
        else
        {
            // Otherwise just fall back to the steam lobby ID
            V_snprintf(m_szDiscordJoinSecret, sizeof(m_szDiscordJoinSecret), "J%lld", lobbyID);
        }

        if (m_bInMap)
        {
            // If the user is in a map then add a valid spectate secret (the same as the join secret only with an 'S')
            // MOM_TODO: Check the `mom_lobby_type` cvar first
            V_snprintf(m_szDiscordSpectateSecret, sizeof(m_szDiscordSpectateSecret), "S%s", &m_szDiscordJoinSecret[1]);
        }
        else
        {
            m_szDiscordSpectateSecret[0] = '\0';
        }
    }
    else
    {
        m_sSteamLobbyID.Clear();
        m_szDiscordPartyId[0] = '\0';
        m_szDiscordJoinSecret[0] = '\0';
    }
}

// Clears out all of the discord fields (optionally can preserve the party ID fields)
void CMomentumDiscord::ClearDiscordFields(bool clearPartyFields /*=true*/)
{
    m_szDiscordState[0] = '\0';
    m_szDiscordDetails[0] = '\0';
    m_iDiscordStartTimestamp = 0;
    m_iDiscordEndTimestamp = 0;
    m_szDiscordLargeImageKey[0] = '\0';
    m_szDiscordLargeImageText[0] = '\0';
    m_szDiscordSmallImageKey[0] = '\0';
    m_szDiscordSmallImageText[0] = '\0';
    m_iDiscordInstance = 0;

    if (clearPartyFields)
    {
        m_iDiscordPartySize = 0;
        m_iDiscordPartyMax = 0;
        m_szDiscordPartyId[0] = '\0';
        m_szDiscordMatchSecret[0] = '\0';
        m_szDiscordSpectateSecret[0] = '\0';
        m_szDiscordJoinSecret[0] = '\0';
    }
}

// Initialize the discord interface
void CMomentumDiscord::DiscordInit()
{
    if (!mom_discord_enable.GetBool())
        return;

    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    handlers.ready = HandleDiscordReady;
    handlers.disconnected = HandleDiscordDisconnected;
    handlers.errored = HandleDiscordError;
    handlers.joinGame = HandleDiscordJoin;
    handlers.spectateGame = HandleDiscordSpectate;
    handlers.joinRequest = HandleDiscordJoinRequest;
    Discord_Initialize(DISCORD_APP_ID, &handlers, 1, MOM_STEAM_ID);
}

// Send an update to discord
void CMomentumDiscord::DiscordUpdate()
{
    if (!mom_discord_enable.GetBool())
        return;

    DiscordRichPresence discordPresence;
    memset(&discordPresence, 0, sizeof(discordPresence));
    discordPresence.state = m_szDiscordState;
    discordPresence.details = m_szDiscordDetails;
    discordPresence.startTimestamp = m_iDiscordStartTimestamp;
    discordPresence.endTimestamp = m_iDiscordEndTimestamp;
    discordPresence.largeImageKey = m_szDiscordLargeImageKey;
    discordPresence.largeImageText = m_szDiscordLargeImageText;
    discordPresence.smallImageKey = m_szDiscordSmallImageKey;
    discordPresence.smallImageText = m_szDiscordSmallImageText;
    discordPresence.partyId = m_szDiscordPartyId;
    discordPresence.partySize = m_iDiscordPartySize;
    discordPresence.partyMax = m_iDiscordPartyMax;
    discordPresence.matchSecret = m_szDiscordMatchSecret;
    discordPresence.spectateSecret = m_szDiscordSpectateSecret;
    discordPresence.joinSecret = m_szDiscordJoinSecret;
    discordPresence.instance = m_iDiscordInstance;
    Discord_UpdatePresence(&discordPresence);
}

// --------------------------- Discord Callbacks ---------------------------- //

// On connect
void CMomentumDiscord::HandleDiscordReady(const DiscordUser *connectedUser)
{
    DevLog("\nDiscord-RPC: connected to user %s#%s - %s\n", connectedUser->username, connectedUser->discriminator,
           connectedUser->userId);
}

// On disconnect
void CMomentumDiscord::HandleDiscordDisconnected(int errcode, const char *message)
{
    DevLog("\nDiscord-RPC: disconnected (%d: %s)\n", errcode, message);
}

// On an error
void CMomentumDiscord::HandleDiscordError(int errcode, const char *message)
{
    DevWarning("\nDiscord-RPC: error (%d: %s)\n", errcode, message);
}

// When a user has been invited to or requested to join a lobby and was granted
void CMomentumDiscord::HandleDiscordJoin(const char *secret)
{
    DevLog("\nDiscord-RPC: join (%s)\n", secret);
    // "connect_lobby" is defined in `mom_lobby_system.cpp`

    // join secret[0] should be a 'J' placed there in `UpdateDiscordPartyIdFromSteam`
    if (secret && secret[0] == 'J')
    {
        // Get Steam Lobby ID and User ID
        CUtlVector<char*> steamIDs;
        V_SplitString(secret, ";", steamIDs);

        if (steamIDs.Count() > 0)
        {
            g_pMomentumDiscord->JoinSteamLobbyFromID(&steamIDs[0][1]);
        }
        else
        {
            // We may want to show the user in a better way that this failed
            DevWarning("Could not join lobby from discord: %s\n", secret);
        }

        steamIDs.PurgeAndDeleteElements();
    }
    else
    {
        DevWarning("Got a bad lobby ID from Discord: %s\n", (secret == nullptr ? "NULL" : secret));
    }
}

// When a user has chosen to spectate through discord
void CMomentumDiscord::HandleDiscordSpectate(const char *secret)
{
    DevLog("\nDiscord-RPC: spectate (%s)\n", secret == nullptr ? "NULL" : secret);

    // spectate secret[0] should be a 'S' placed there in `UpdateDiscordPartyIdFromSteam`
    if (secret && secret[0] == 'S')
    {
        // PurgeAndDeleteElements() must be called on this when done otherwise memory will leak
        CUtlVector<char*> steamIDs;
        V_SplitString(secret, ";", steamIDs);

        if (steamIDs.Count() != 2)
        {
            DevWarning("Did not get enough information from discord to spectate: %s\n", secret);
            steamIDs.PurgeAndDeleteElements();
            return;
        }

        V_strncpy(g_pMomentumDiscord->m_szSpectateTargetLobby, &steamIDs[0][1], DISCORD_MAX_BUFFER_SIZE);
        V_strncpy(g_pMomentumDiscord->m_szSpectateTargetUser, steamIDs[1], DISCORD_MAX_BUFFER_SIZE);

        // Clean up memory (do not return before doing this!)
        steamIDs.PurgeAndDeleteElements();

        // If we did not get enough info from discord
        if (!g_pMomentumDiscord->m_szSpectateTargetLobby[0] || !g_pMomentumDiscord->m_szSpectateTargetUser[0])
        {
            DevWarning("Error in lobby or user ID\n");
            return;
        }

        // Try to get our target's current map
        g_pMomentumDiscord->m_ulSpectateTargetUser = V_atoui64(g_pMomentumDiscord->m_szSpectateTargetUser);
        CSteamID targetPlayerSteamID = CSteamID(g_pMomentumDiscord->m_ulSpectateTargetUser);
        const char *targetMapName = g_pMomentumDiscord->GetMapOfPlayerFromSteamID(&targetPlayerSteamID);

        bool inSameLobby = g_pMomentumDiscord->m_sSteamLobbyID.ConvertToUint64() ==
                           V_atoui64(g_pMomentumDiscord->m_szSpectateTargetLobby);
        bool inSameMap = g_pMomentumDiscord->m_bInMap && g_pMomentumDiscord->MapName()[0] && targetMapName &&
                         !V_strcmp(targetMapName, g_pMomentumDiscord->MapName());

        // Set our starting state before we go into the SpectateTargetFromDiscord state machine
        if (inSameLobby && inSameMap)
        {
            g_pMomentumDiscord->m_kJoinSpectateState = WaitOnNone;
        }
        else if (inSameLobby)
        {
            g_pMomentumDiscord->m_kJoinSpectateState = WaitOnMap;
        }
        else if (inSameMap)
        {
            g_pMomentumDiscord->m_kJoinSpectateState = WaitOnLobby;
        }
        else
        {
            g_pMomentumDiscord->m_kJoinSpectateState = WaitOnLobbyAndMap;
        }

        g_pMomentumDiscord->SpectateTargetFromDiscord();
    }
    else
    {
        DevWarning("Got a bad lobby ID from Discord\n");
    }
}

// When a different user is requesting to join the current user's lobby
void CMomentumDiscord::HandleDiscordJoinRequest(const DiscordUser *request)
{
    DevLog("\nDiscord-RPC: join request from %s#%s - %s\n", request->username, request->discriminator, request->userId);
}

// -------------------------------------------------------------------------- //

static CMomentumDiscord s_Discord;
CMomentumDiscord *g_pMomentumDiscord = &s_Discord;
