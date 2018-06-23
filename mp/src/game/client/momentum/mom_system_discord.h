#pragma once

#include "cbase.h"
#include "steam/steam_api.h"
#include "GameEventListener.h"
#include "discord_rpc.h"

// Set by the discord API as the max
#define DISCORD_MAX_BUFFER_SIZE 128

// A class to manage the Discord Rich Presence feature
// Uses the discord-rpc library
// Libary source code is here: https://github.com/discordapp/discord-rpc
// Docs are here: https://discordapp.com/developers/docs/rich-presence/how-to
class CMomentumDiscord : public CAutoGameSystemPerFrame, public CGameEventListener {

public:
    CMomentumDiscord(const char *pName);

    // CAutoGameSystemPerFrame
    void PostInit() OVERRIDE;
    void LevelInitPreEntity() OVERRIDE;
    void LevelInitPostEntity() OVERRIDE;
    void LevelShutdownPreEntity() OVERRIDE;
    void Update(float frametime) OVERRIDE;
    void Shutdown() OVERRIDE;

    // CGameEventListener
    void FireGameEvent(IGameEvent* event) OVERRIDE;

    // Steam Matchmaking API
    // Docs: https://partner.steamgames.com/doc/api/ISteamMatchmaking#callbacks
    STEAM_CALLBACK(CMomentumDiscord, HandleLobbyEnter, LobbyEnter_t); // We entered this lobby (or failed to enter)
    STEAM_CALLBACK(CMomentumDiscord, HandleLobbyDataUpdate, LobbyDataUpdate_t); // Something was updated for the lobby's data
    STEAM_CALLBACK(CMomentumDiscord, HandleLobbyChatUpdate, LobbyChatUpdate_t); // Something was updated for the lobby's data

    // Custom members
    enum JoinSpectateState {
        Null, WaitOnLobbyAndMap, WaitOnMap, WaitOnLobby, WaitOnNone
    };
    CSteamID m_sSteamLobbyID;
    CSteamID m_sSteamUserID;
    bool m_bInMap;
    
    // Custom methods
    static const char* GetMapOfPlayerFromSteamID(CSteamID* steamID);
    static bool JoinSteamLobbyFromID(const char* lobbyID);
    static bool JoinMapFromUserSteamID(uint64 steamID);
    static void SpecPlayerFromSteamId(const char* steamID);
    void ClearDiscordFields(bool clearPartyFields=true);
    void GetSteamUserID();
    void OnSteamLobbyUpdate();
    void SpectateTargetFromDiscord();

    // To handle spectating a player when we are on a different map or not in
    // the same lobby we need to hold off spectating until we change the map and 
    // join the correct lobby. These members help with that functionality
    char m_szSpectateTargetLobby[DISCORD_MAX_BUFFER_SIZE];
    char m_szSpectateTargetUser[DISCORD_MAX_BUFFER_SIZE];
    uint64 m_ulSpectateTargetUser;
    JoinSpectateState m_kJoinSpectateState;

    // Discord Presence Payload Fields
    // https://discordapp.com/developers/docs/rich-presence/how-to#updating-presence
    char m_szDiscordState[DISCORD_MAX_BUFFER_SIZE];           // the user's current party status
    char m_szDiscordDetails[DISCORD_MAX_BUFFER_SIZE];         // what the player is currently doing
    int64_t m_iDiscordStartTimestamp;                     // epoch seconds for game start - including will show time as "elapsed"
    int64_t m_iDiscordEndTimestamp;                       // epoch seconds for game end - including will show time as "remaining"
    char m_szDiscordLargeImageKey[DISCORD_MAX_BUFFER_SIZE];   // name of the uploaded image for the large profile artwork
    char m_szDiscordLargeImageText[DISCORD_MAX_BUFFER_SIZE];  // tooltip for the largeImageKey
    char m_szDiscordSmallImageKey[DISCORD_MAX_BUFFER_SIZE];   // name of the uploaded image for the small profile artwork
    char m_szDiscordSmallImageText[DISCORD_MAX_BUFFER_SIZE];  // tootltip for the smallImageKey
    char m_szDiscordPartyId[DISCORD_MAX_BUFFER_SIZE];         // id of the player's party, lobby, or group
    int m_iDiscordPartySize;                              // current size of the player's party, lobby, or group 1
    int m_iDiscordPartyMax;                               // maximum size of the player's party, lobby, or group 5
    char m_szDiscordMatchSecret[DISCORD_MAX_BUFFER_SIZE];     // [deprecated Notify Me feature, may be re-used in future]
    char m_szDiscordSpectateSecret[DISCORD_MAX_BUFFER_SIZE];  // unique hashed string for Spectate button
    char m_szDiscordJoinSecret[DISCORD_MAX_BUFFER_SIZE];      // unique hashed string for chat invitations and Ask to Join
    int8_t m_iDiscordInstance;                            // [deprecated Notify Me feature, may be re-used in future]

    // Payload helper fields
    char m_szInMenusStatusString[DISCORD_MAX_BUFFER_SIZE];   // Status string to use when player is in the menu
    char m_szInMenusLargeImage[DISCORD_MAX_BUFFER_SIZE];     // Large image key to use when player is in the menus

    // Static vars
    static const char* s_pDiscordAppID;
    static const char* s_pSteamAppID;

private:
    // Custom members
    int m_iUpdateFrame;

    // Custom methods
    void DiscordInit();
    void DiscordUpdate();
    void UpdateDiscordPartyIdFromSteam();
    void UpdateLobbyNumbers();

    // Discord callbacks
    static void HandleDiscordReady(const DiscordUser* connectedUser);
    static void HandleDiscordDisconnected(int errcode, const char* message);
    static void HandleDiscordError(int errcode, const char* message);
    static void HandleDiscordJoin(const char* secret);
    static void HandleDiscordSpectate(const char* secret);
    static void HandleDiscordJoinRequest(const DiscordUser* request);
};

// Make this class available to other client classes
extern CMomentumDiscord *g_pMomentumDiscord;
