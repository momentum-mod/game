#ifndef MOM_SHAREDDEFS_H
#define MOM_SHAREDDEFS_H
#ifdef _WIN32
#pragma once
#endif

#ifndef GHOST_SERVER
#include "const.h"
#if defined(GAME_DLL) || defined(CLIENT_DLL)
#include "shareddefs.h"
#endif
#endif
// Main Version (0 is prealpha, 1 is alpha, 2 is beta and 3 is release)​.Main feature push (increment by one for each)​.​Small commits or hotfixes​
// If you have any doubts, please refer to http://semver.org/
#define MOM_CURRENT_VERSION "0.7.2"


// Gamemode for momentum
enum GAME_MODE
{
    GAMEMODE_UNKNOWN = 0, // Non-recognized map (no info ents in it)
    GAMEMODE_SURF = 1,
    GAMEMODE_BHOP = 2,
    GAMEMODE_KZ = 3,
    GAMEMODE_RJ = 4,
    GAMEMODE_TRICKSURF = 5,
    GAMEMODE_TRIKZ = 6,
    // MOM_TODO: etc

    // NOTE NOTE: IF YOU UPDATE THIS, UPDATE MOMENTUM.FGD's "GameTypes" BASECLASS!

};

const char * const g_szGameModes[] = {
    "#MOM_NotApplicable",
    "#MOM_GameType_Surf",
    "#MOM_GameType_Bhop",
    "#MOM_GameType_KZ",
    "#MOM_GameType_RJ",
    "#MOM_GameType_Tricksurf",
    "#MOM_GameType_Trikz"
};

// Run Flags
enum RUN_FLAG
{
    RUNFLAG_NONE = 0,
    RUNFLAG_SCROLL = 1 << 0,
    RUNFLAG_W_ONLY = 1 << 1,
    RUNFLAG_HSW = 1 << 2,
    RUNFLAG_SW = 1 << 3,
    RUNFLAG_BW = 1 << 4,
    RUNFLAG_BONUS = 1 << 5
    //MOM_TODO: Figure out the rest
};

enum MAP_UPLOAD_STATUS
{
    STATUS_UNKNOWN = -1,
    MAP_APPROVED,
    MAP_PENDING,
    MAP_NEEDS_REVISION,
    MAP_PRIVATE_TESTING,
    MAP_PUBLIC_TESTING,
    MAP_READY_FOR_RELEASE,
    MAP_REJECTED,
    MAP_REMOVED,
};

enum MAP_CREDIT_TYPE
{
    CREDIT_UNKNOWN = -1,
    CREDIT_AUTHOR,
    CREDIT_TESTER,
    CREDIT_SPECIAL_THANKS,
};

enum USER_PERMISSIONS
{
    USER_VERIFIED = 1 << 0,
    USER_MAPPER = 1 << 1,
    USER_MODERATOR = 1 << 2,
    USER_ADMIN = 1 << 3,
};

enum USER_BANS
{
    USER_BANNED_ALIAS = 1 << 0,
    USER_BANNED_AVATAR = 1 << 1,
    USER_BANNED_LEADERBOARDS = 1 << 2,
};

enum LOBBY_MSG_TYPE
{
    LOBBY_UPDATE_MEMBER_JOIN = 0,        // Joined the lobby
    LOBBY_UPDATE_MEMBER_JOIN_MAP,        // Joined your map
    LOBBY_UPDATE_MEMBER_LEAVE,           // Left your lobby
    LOBBY_UPDATE_MEMBER_LEAVE_MAP,       // Left your map
};

enum SPECTATE_MSG_TYPE
{
    SPEC_UPDATE_JOIN = 0,           // Started spectating
    SPEC_UPDATE_CHANGETARGET,    // Is now spectating someone else
    SPEC_UPDATE_STOP,           // Stopped spectating; respawned
    SPEC_UPDATE_LEAVE       // This player left the map/lobby
};

#define PANEL_TIMES "times"
#define PANEL_REPLAY "replaycontrols"
#define PANEL_LOBBY_MEMBERS "LobbyMembers"

#define MOM_COLORIZATION_CHECK_FREQUENCY 0.1f

#define END_RECORDING_DELAY 1.0f //Delay the ending by this amount of seconds
#define START_TRIGGER_TIME_SEC 2.0f //We only want this amount in seconds of being in the start trigger

//buffers for cstr variables
#define BUFSIZETIME (sizeof("00:00:00.000")+1)
#define BUFSIZELOCL (256)//Buffer size for localization/Max length for localized string
#define BUFSIZESHORT 10

//Localization of tokens
//Checks to see if the token exists, and if so, localizes it into output. Otherwise
//it's just the token value. This exists to prevent null localization tokens.
#define FIND_LOCALIZATION(output, token) \
    Q_wcsncpy(output, g_pVGuiLocalize->Find(token) ? g_pVGuiLocalize->Find(token) : L##token , sizeof(output))


//Localizes a token to an ansi output array, under a name macro
#define LOCALIZE_TOKEN(name, token, output)\
    wchar_t *unicode_##name = g_pVGuiLocalize->Find(token);\
    g_pVGuiLocalize->ConvertUnicodeToANSI(unicode_##name ? unicode_##name : L##token, output, sizeof(output));


//Takes input ansi and converts, using g_pVGuiLocalize, to unicode
#define ANSI_TO_UNICODE(ansi, unicode) \
    g_pVGuiLocalize->ConvertANSIToUnicode(ansi, unicode, sizeof(unicode));

// Fires an IGameEvent game-wide, so that anything that is listening can hear it
#define FIRE_GAME_WIDE_EVENT(eventName) \
    IGameEvent *eventServer = gameeventmanager->CreateEvent(eventName, true); \
    IGameEvent *eventClient = gameeventmanager->CreateEvent(eventName, true); \
    if (eventServer && eventClient) \
    { gameeventmanager->FireEvent(eventServer, true); gameeventmanager->FireEventClientSide(eventClient); }

// Creates a convar with a callback function
#define MAKE_CONVAR_C(name, defaultval, flags, desc, minVal, maxVal, callback)                                                \
    ConVar name(#name, defaultval, flags, desc, true, minVal, true, maxVal, callback)

//Creates a convar, mainly used for MAKE_TOGGLE
#define MAKE_CONVAR(name, defaultval, flags, desc, minVal, maxVal)                                                            \
    ConVar name(#name, defaultval, flags, desc, true, minVal, true, maxVal)

//Creates a CONVAR with 0 as the minimum value, and 1 as the max value. Useful for toggle variables.
#define MAKE_TOGGLE_CONVAR(name, defaultval, flags, desc) MAKE_CONVAR(name, defaultval, flags, desc, 0, 1)

#define MAKE_TOGGLE_CONVAR_C(name, defaultval, flags, desc, callback) MAKE_CONVAR_C(name, defaultval, flags, desc, 0, 1, callback)

//Flags for a HUD cvar (usually)
#define FLAG_HUD_CVAR (FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_REPLICATED)

#define MAX_STAGES 64

#define CHECK_STEAM_API(steam_interface) \
    if (!steam_interface) { \
    Warning("%s->%s(): %d -- Steam API Interface %s could not be loaded! You may be offline or Steam may not be running!\n",  \
    __FILE__, __FUNCTION__, __LINE__, #steam_interface); return; }

#define MAP_FOLDER "maps"
#define RECORDING_PATH "replays"
#define RECORDING_ONLINE_PATH "online"
#define EXT_ZONE_FILE ".zon"
#define EXT_RECORDING_FILE ".mrf"

// MOM_TODO: Replace this with the custom player model
#define ENTITY_MODEL "models/player/player_shape_base.mdl"

// Change these if you want to change the flashlight sound
#define SND_FLASHLIGHT_ON "CSPlayer.FlashlightOn"
#define SND_FLASHLIGHT_OFF "CSPlayer.FlashlightOff"

#define LOBBY_DATA_MAP "map"
#define LOBBY_DATA_APPEARANCE "appearance"
#define LOBBY_DATA_TYPING "isTyping"
#define LOBBY_DATA_SPEC_TARGET "specTargetID"
#define LOBBY_DATA_IS_SPEC "isSpectating"

static const unsigned long long MOM_STEAM_GROUP_ID64 = 103582791441609755;

#endif // MOM_SHAREDDEFS_H
