#include "cbase.h"
#include "fmtstr.h"
#include "mom_system_discord.h"
#include "mom_shareddefs.h"
#include <time.h>

#include "tier0/memdbgon.h"

// MOM_TODO: Change this to the official Discord App ID (currently it is under Connor's test account)
#define DISCORD_APP_ID "452297955718987776"
#define MOM_STEAM_ID "669270"

// How many frames to wait before updating discord
// (some things are still updated each frame such as checking callbacks)
#ifndef DISCORD_FRAME_UPDATE_FREQ
    #define DISCORD_FRAME_UPDATE_FREQ 600
#endif

#ifdef DEBUG
CON_COMMAND(mom_discord_debug, "Help debug discord integration\n") {
    DevLog("Party ID: %s\n", g_pMomentumDiscord->m_szDiscordPartyId);
    DevLog("Spectate: %s\n", g_pMomentumDiscord->m_szDiscordSpectateSecret);
    DevLog("Join:     %s\n", g_pMomentumDiscord->m_szDiscordJoinSecret);
}
#endif

CMomentumDiscord::CMomentumDiscord(const char *pName) {
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

    V_strncpy(m_szInMenusStatusString, "In Menus", DISCORD_MAX_BUFFER_SIZE);
    V_strncpy(m_szInMenusLargeImage, "mom", DISCORD_MAX_BUFFER_SIZE);
};

// ---------------------------- CAutoGameSystem ----------------------------- //

// Called once on game start
void CMomentumDiscord::PostInit() {
    ListenForGameEvent("lobby_leave");

    DiscordInit();

    // Default discord information
    V_strncpy(m_szDiscordState, m_szInMenusStatusString, DISCORD_MAX_BUFFER_SIZE);
    V_strncpy(m_szDiscordLargeImageKey, m_szInMenusLargeImage, DISCORD_MAX_BUFFER_SIZE);

    GetSteamUserID();
}

// Called each time a map is joined before entities are initialized
void CMomentumDiscord::LevelInitPreEntity() {
    m_bInMap = true;

    ConVarRef gm("mom_gamemode");
    switch (gm.GetInt()) {
        case MOMGM_SURF:
            V_strncpy(m_szDiscordState, "Surfing", DISCORD_MAX_BUFFER_SIZE);
            break;
        case MOMGM_BHOP:
            V_strncpy(m_szDiscordState, "Bhopping", DISCORD_MAX_BUFFER_SIZE);
            break;
        case MOMGM_SCROLL:
            V_strncpy(m_szDiscordState, "Scrolling", DISCORD_MAX_BUFFER_SIZE);
            break;
        case MOMGM_UNKNOWN:
        default:
            V_strncpy(m_szDiscordState, "Playing", DISCORD_MAX_BUFFER_SIZE);
            break;
    }

    V_strncpy(m_szDiscordDetails, MapName(), DISCORD_MAX_BUFFER_SIZE);
    V_strncpy(m_szDiscordLargeImageText, MapName(), DISCORD_MAX_BUFFER_SIZE);
    m_iDiscordStartTimestamp = time(0);
    // MOM_TODO: Add more / better information for discord
    // (a badge for the small image, better large images such as an icon for each game type, current map stage, etc.)
}

// Called each time a map is joined after entities are initialized
void CMomentumDiscord::LevelInitPostEntity() {
    // Check to see if we are joining this map to spectate a player
    if (m_kJoinSpectateState == WaitOnMap) {
        m_kJoinSpectateState = WaitOnNone;
        SpectateTargetFromDiscord();
    }
}

// Called each time a map is left
void CMomentumDiscord::LevelShutdownPreEntity() {
    m_bInMap = false;
    ClearDiscordFields(false); // Pass false to retain party/lobby related fields
    V_strncpy(m_szDiscordState, m_szInMenusStatusString, DISCORD_MAX_BUFFER_SIZE);
    V_strncpy(m_szDiscordLargeImageKey, m_szInMenusLargeImage, DISCORD_MAX_BUFFER_SIZE);
}

// Called every frame
void CMomentumDiscord::Update(float frametime) {
    if (m_iUpdateFrame++ == DISCORD_FRAME_UPDATE_FREQ) {
        // Stuff that can be done every ~1 second
        m_iUpdateFrame = 0;
        UpdateDiscordPartyIdFromSteam();
        UpdateLobbyNumbers();
        DiscordUpdate();
    }

    // Do the rest of this stuff every frame
    #ifdef DISCORD_DISABLE_IO_THREAD
        Discord_UpdateConnection();
    #endif
    Discord_RunCallbacks();
}

// Called on game quit
void CMomentumDiscord::Shutdown() {
    DevLog("Shutting down Discord\n");
    Discord_Shutdown();
}

// -------------------------- Steam API Callbacks --------------------------- //

// When a user joins a lobby
void CMomentumDiscord::HandleLobbyEnter(LobbyEnter_t* pParam) {
    if (pParam->m_EChatRoomEnterResponse != k_EChatRoomEnterResponseSuccess || pParam->m_ulSteamIDLobby == 0) {
        DevWarning("Failed to enter room! Error code: %i\n", pParam->m_EChatRoomEnterResponse);
        return;
    }

    m_sSteamLobbyID = CSteamID(pParam->m_ulSteamIDLobby);
    UpdateDiscordPartyIdFromSteam();
}

void CMomentumDiscord::HandleLobbyDataUpdate(LobbyDataUpdate_t* pParam) {
    if (CSteamID(pParam->m_ulSteamIDLobby) != m_sSteamLobbyID) {
        m_sSteamLobbyID = CSteamID(pParam->m_ulSteamIDLobby);
        UpdateDiscordPartyIdFromSteam();
    }

    OnSteamLobbyUpdate();
}

void CMomentumDiscord::HandleLobbyChatUpdate(LobbyChatUpdate_t* pParam) {
    if (pParam->m_ulSteamIDLobby != m_sSteamLobbyID.ConvertToUint64()) {
        m_sSteamLobbyID = CSteamID(pParam->m_ulSteamIDLobby);
        UpdateDiscordPartyIdFromSteam();
    }

    OnSteamLobbyUpdate();
}

// --------------------------- CGameEventListner ---------------------------- //

// Called when there is a game event we are listening for
// Subscribe to events in `PostInit`
void CMomentumDiscord::FireGameEvent(IGameEvent* event) {
    //if (!Q_strcmp(event->GetName(), "lobby_leave")) {
        m_sSteamLobbyID.Clear();
        m_iDiscordPartySize = 0;
        m_iDiscordPartyMax = 0;
        m_szDiscordPartyId[0] = '\0';
        m_szDiscordJoinSecret[0] = '\0';
        m_szDiscordSpectateSecret[0] = '\0';
    //}
}

// ----------------------------- Custom Methods ----------------------------- //

// Gets the current user's steam ID
void CMomentumDiscord::GetSteamUserID() {
    CHECK_STEAM_API(SteamUser());
    m_sSteamUserID = SteamUser()->GetSteamID();
}

// Get the current map of a player from their steam ID
const char* CMomentumDiscord::GetMapOfPlayerFromSteamID(CSteamID* steamID) {
    if (!SteamMatchmaking() || !g_pMomentumDiscord->m_sSteamLobbyID.IsValid()) {
        return "";
    }
    else {
        return SteamMatchmaking()->GetLobbyMemberData(g_pMomentumDiscord->m_sSteamLobbyID, *steamID, LOBBY_DATA_MAP);
    }
}

// Joins a lobby from the Steam Lobby ID
// returns true if we had to change the lobby
bool CMomentumDiscord::JoinSteamLobbyFromID(const char* lobbyID) {
    if (g_pMomentumDiscord->m_sSteamLobbyID.IsValid() && !V_strcmp(lobbyID, CFmtStrN<DISCORD_MAX_BUFFER_SIZE>("%lld", g_pMomentumDiscord->m_sSteamLobbyID.ConvertToUint64()))) {
        // If we are already in the lobby, don't join
        return false;
    }
    else if (g_pMomentumDiscord->m_sSteamLobbyID.IsValid()) {
        // If we are in a different lobby we need to leave
        DISPATCH_CON_COMMAND("mom_lobby_leave", "mom_lobby_leave");
    }

    // Finally join the new lobby
    DISPATCH_CON_COMMAND("connect_lobby", CFmtStrN<DISCORD_MAX_BUFFER_SIZE>("connect_lobby %s", lobbyID));

    return true;
}

// Joins the same map as the player referenced to by `steamID`
// returns true if we had to change maps or couldn't get the map
bool CMomentumDiscord::JoinMapFromUserSteamID(uint64 steamID) {
    CSteamID targetPlayerSteamID = CSteamID(steamID);
    const char* targetMapName = GetMapOfPlayerFromSteamID(&targetPlayerSteamID);
    
    if (!targetMapName[0]) {
        return false;
    }

    if (g_pMomentumDiscord->MapName() && !V_strcmp(targetMapName, g_pMomentumDiscord->MapName())) {
        // Already on the same map
        return false;
    } else {
        DISPATCH_CON_COMMAND("map", CFmtStrN<DISCORD_MAX_BUFFER_SIZE>("map %s", targetMapName))
    }

    return true;
}

// When there is an update from the steam lobby API
void CMomentumDiscord::OnSteamLobbyUpdate() {
    // Check to see if we are waiting to spectate a player
    if (m_kJoinSpectateState == WaitOnLobbyAndMap) {
        m_kJoinSpectateState = WaitOnMap;
        SpectateTargetFromDiscord();
    } else if (m_kJoinSpectateState == WaitOnLobby) {
        m_kJoinSpectateState = WaitOnNone;
        SpectateTargetFromDiscord();
    }
}

// Handle spectating a player (spectating is more complicated than it would seem)
// This method acts as a state machine as we may need to wait for things to happen
// such as joining a steam lobby, and/or joining a map, before we can spectate
void CMomentumDiscord::SpectateTargetFromDiscord() {
    switch (m_kJoinSpectateState) {
        case WaitOnLobbyAndMap:
            if (!JoinSteamLobbyFromID(m_szSpectateTargetLobby)) {
                m_kJoinSpectateState = WaitOnMap;
                SpectateTargetFromDiscord();
            }
            break;        

        case WaitOnLobby:
            if (!JoinSteamLobbyFromID(m_szSpectateTargetLobby)) {
                m_kJoinSpectateState = WaitOnNone;
                SpectateTargetFromDiscord();
            }
            break;
        
        case WaitOnMap:
            if (!JoinMapFromUserSteamID(m_ulSpectateTargetUser)) {
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
void CMomentumDiscord::SpecPlayerFromSteamId(const char* steamID) {
    DevWarning("CMomentumDiscord::SpecPlayerFromSteamId(%s)\n", steamID);

    // MOM_TODO: We probably should check if the target player is already spectating someone, then set the spectate target them instead
    // Or we could adjust the `mom_spectate` command to handle this automatically
    DISPATCH_CON_COMMAND("mom_spectate", CFmtStrN<DISCORD_MAX_BUFFER_SIZE>("mom_spectate %s", steamID));
}

// Updates the current and max party size based upon the current lobby
void CMomentumDiscord::UpdateLobbyNumbers() {
    CHECK_STEAM_API(SteamMatchmaking());
    if (m_sSteamLobbyID.IsValid()) {
        m_iDiscordPartySize = SteamMatchmaking()->GetNumLobbyMembers(m_sSteamLobbyID);
        m_iDiscordPartyMax = SteamMatchmaking()->GetLobbyMemberLimit(m_sSteamLobbyID);
    }
}

// Keeps the discord party ID and secrets up to date from the steam data
void CMomentumDiscord::UpdateDiscordPartyIdFromSteam() {
    if (m_sSteamLobbyID.IsValid()) {
        // The party ID is just the Steam Lobby ID
        V_strncpy(m_szDiscordPartyId, CFmtStrN<DISCORD_MAX_BUFFER_SIZE>("%lld", m_sSteamLobbyID.ConvertToUint64()), DISCORD_MAX_BUFFER_SIZE);


        if (!m_sSteamUserID.IsValid()) {
            // If the steam user ID is not valid, try and get it
            GetSteamUserID();
        }

        // We could do something to further "encrypt" the secrets but it's probably fine
        if (m_sSteamUserID.IsValid()) {
            // If we found a vaid steam user ID add it into the secret
            V_strncpy(m_szDiscordJoinSecret, CFmtStrN<DISCORD_MAX_BUFFER_SIZE>("J%lld;%lld",
                m_sSteamLobbyID.ConvertToUint64(),
                m_sSteamUserID.ConvertToUint64()),
            DISCORD_MAX_BUFFER_SIZE);
        }
        else {
            // Otherwise just fall back to the steam lobby ID
            V_strncpy(m_szDiscordJoinSecret, CFmtStrN<DISCORD_MAX_BUFFER_SIZE>("J%lld",
                m_sSteamLobbyID.ConvertToUint64()),
            DISCORD_MAX_BUFFER_SIZE);
        }

        if (m_bInMap) {
            // If the user is in a map then add a valid spectate secret (the same as the join secret only with an 'S')
            // MOM_TODO: Check the `mom_lobby_type` cvar first
            m_szDiscordSpectateSecret[0] = 'S';
            V_strncpy(&m_szDiscordSpectateSecret[1], &m_szDiscordJoinSecret[1], DISCORD_MAX_BUFFER_SIZE-1);
        }
        else {
            m_szDiscordSpectateSecret[0] = '\0';
        }
    }
    else {
        m_sSteamLobbyID.Clear();
        m_szDiscordPartyId[0] = '\0';
        m_szDiscordJoinSecret[0] = '\0';
    }
}

// Clears out all of the discord fields (optionally can preserve the party ID fields)
void CMomentumDiscord::ClearDiscordFields(bool clearPartyFields /*=true*/) {
    m_szDiscordState[0] = '\0';
    m_szDiscordDetails[0] = '\0';
    m_iDiscordStartTimestamp = 0;
    m_iDiscordEndTimestamp = 0;
    m_szDiscordLargeImageKey[0] = '\0';
    m_szDiscordLargeImageText[0] = '\0';
    m_szDiscordSmallImageKey[0] = '\0';
    m_szDiscordSmallImageText[0] = '\0';
    m_iDiscordInstance = 0;

    if (clearPartyFields) {
        m_iDiscordPartySize = 0;
        m_iDiscordPartyMax = 0;
        m_szDiscordPartyId[0] = '\0';
        m_szDiscordMatchSecret[0] = '\0';
        m_szDiscordSpectateSecret[0] = '\0';
        m_szDiscordJoinSecret[0] = '\0';
    }
}

// Initialize the discord interface
void CMomentumDiscord::DiscordInit() {
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    handlers.ready = CMomentumDiscord::HandleDiscordReady;
    handlers.disconnected = CMomentumDiscord::HandleDiscordDisconnected;
    handlers.errored = CMomentumDiscord::HandleDiscordError;
    handlers.joinGame = CMomentumDiscord::HandleDiscordJoin;
    handlers.spectateGame = CMomentumDiscord::HandleDiscordSpectate;
    handlers.joinRequest = CMomentumDiscord::HandleDiscordJoinRequest;
    Discord_Initialize(DISCORD_APP_ID, &handlers, 1, MOM_STEAM_ID);
}


// Send an update to discord
void CMomentumDiscord::DiscordUpdate() {
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
void CMomentumDiscord::HandleDiscordReady(const DiscordUser* connectedUser) {
    DevLog("\nDiscord-RPC: connected to user %s#%s - %s\n",
        connectedUser->username,
        connectedUser->discriminator,
        connectedUser->userId);
}

// On disconnect
void CMomentumDiscord::HandleDiscordDisconnected(int errcode, const char* message) {
    DevLog("\nDiscord-RPC: disconnected (%d: %s)\n", errcode, message);
}

// On an error
void CMomentumDiscord::HandleDiscordError(int errcode, const char* message) {
    DevLog("\nDiscord-RPC: error (%d: %s)\n", errcode, message);
}

// When a user has been invited to or requested to join a lobby and was granted
void CMomentumDiscord::HandleDiscordJoin(const char* secret) {
    DevLog("\nDiscord-RPC: join (%s)\n", secret);
    // "connect_lobby" is defined in `mom_lobby_system.cpp`

    // join secret[0] should be a 'J' placed there in `UpdateDiscordPartyIdFromSteam`
    if (secret && secret[0] == 'J') {
        // Get Steam Lobby ID and User ID
        CUtlVector<char*, CUtlMemory<char*, int>> steamIDs;
        V_SplitString(secret, ";", steamIDs);

        if (steamIDs.Count() > 0) {
            JoinSteamLobbyFromID(&steamIDs[0][1]);
        }
        else {
            // We may want to show the user in a better way that this failed
            DevWarning("Could not join lobby from discord: %s\n", secret);
        }

        steamIDs.PurgeAndDeleteElements();
    }
    else {
        DevWarning("Got a bad lobby ID from Discord: %s\n", (secret == nullptr ? "NULL" : secret));
    }
}

// When a user has chosen to spectate through discord
void CMomentumDiscord::HandleDiscordSpectate(const char* secret) {
    DevLog("\nDiscord-RPC: spectate (%s)\n", secret == nullptr ? "NULL" : secret);

    // spectate secret[0] should be a 'S' placed there in `UpdateDiscordPartyIdFromSteam`
    if (secret && secret[0] == 'S') {
        CUtlVector<char*, CUtlMemory<char*, int>> steamIDs; // PurgeAndDeleteElements() must be called on this when done otherwise memory will leak
        V_SplitString(secret, ";", steamIDs);

        if (steamIDs.Count() != 2) {
            DevWarning("Did not get enough information from discord to spectate: %s\n", secret);
            steamIDs.PurgeAndDeleteElements();
            return;
        }

        V_strncpy(g_pMomentumDiscord->m_szSpectateTargetLobby, &steamIDs[0][1], DISCORD_MAX_BUFFER_SIZE);
        V_strncpy(g_pMomentumDiscord->m_szSpectateTargetUser, steamIDs[1], DISCORD_MAX_BUFFER_SIZE);

        // Clean up memory (do not return before doing this!)
        steamIDs.PurgeAndDeleteElements();

        // If we did not get enough info from discord
        if (!g_pMomentumDiscord->m_szSpectateTargetLobby[0] || !g_pMomentumDiscord->m_szSpectateTargetUser[0]) {
            DevWarning("Error in lobby or user ID\n");
            return;
        }

        // Try to get our target's current map
        g_pMomentumDiscord->m_ulSpectateTargetUser = V_atoui64(g_pMomentumDiscord->m_szSpectateTargetUser);
        CSteamID targetPlayerSteamID = CSteamID(g_pMomentumDiscord->m_ulSpectateTargetUser);
        const char* targetMapName = GetMapOfPlayerFromSteamID(&targetPlayerSteamID);

        bool inSameLobby = g_pMomentumDiscord->m_sSteamLobbyID.ConvertToUint64() == V_atoui64(g_pMomentumDiscord->m_szSpectateTargetLobby);
        bool inSameMap = g_pMomentumDiscord->m_bInMap && g_pMomentumDiscord->MapName()[0] && targetMapName && !V_strcmp(targetMapName, g_pMomentumDiscord->MapName());

        // Set our starting state before we go into the SpectateTargetFromDiscord state machine
        if (inSameLobby && inSameMap) {
            g_pMomentumDiscord->m_kJoinSpectateState = WaitOnNone;
        } else if (inSameLobby) {
            g_pMomentumDiscord->m_kJoinSpectateState = WaitOnMap;
        } else if (inSameMap) {
            g_pMomentumDiscord->m_kJoinSpectateState = WaitOnLobby;
        } else {
            g_pMomentumDiscord->m_kJoinSpectateState = WaitOnLobbyAndMap;
        }

        g_pMomentumDiscord->SpectateTargetFromDiscord();
    }
    else {
        DevWarning("Got a bad lobby ID from Discord\n");
    }
}

// When a differnt user is requesting to join the current user's lobby
void CMomentumDiscord::HandleDiscordJoinRequest(const DiscordUser* request) {
    DevLog("\nDiscord-RPC: join request from %s#%s - %s\n",
        request->username,
        request->discriminator,
        request->userId);

    // Auto-accept party requests for now
    // MOM_TODO: Prompt the player some how to deal with this, but we don't want to interrupt a run
    Discord_Respond(request->userId, DISCORD_REPLY_YES);
}

// -------------------------------------------------------------------------- //

static CMomentumDiscord s_Discord("CMomentumDiscord");
CMomentumDiscord *g_pMomentumDiscord = &s_Discord;
