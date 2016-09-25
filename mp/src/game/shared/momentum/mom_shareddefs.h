#ifndef MOM_SHAREDDEFS_H
#define MOM_SHAREDDEFS_H
#ifdef _WIN32
#pragma once
#endif

#include "const.h"
#include "shareddefs.h"

// Gamemode for momentum
typedef enum MOMGM
{
    MOMGM_UNKNOWN = 0,
    MOMGM_SURF,
    MOMGM_BHOP,
    MOMGM_SCROLL,
    MOMGM_ALLOWED, //not "official gamemode" but must be allowed for other reasons

} GAMEMODES;

// Run Flags
typedef enum FLAGS
{
    RUNFLAG_NONE = 0,
    RUNFLAG_SCROLL = 1 << 0,
    RUNFLAG_W_ONLY = 1 << 1,
    RUNFLAG_HSW = 1 << 2,
    RUNFLAG_SW = 1 << 3,
    RUNFLAG_BW = 1 << 4,
    RUNFLAG_BONUS = 1 << 5
    //MOM_TODO: Figure out the rest
} RUN_FLAG;

#define PANEL_TIMES "times"
#define IN_TIMES (1<<26)

// Main Version (0 is prealpha, 1 is alpha, 2 is beta and 3 is release)​.Main feature push (increment by one for each)​.​Small commits or hotfixes​
// When editing this, remember to also edit version.txt on the main dir of the repo
// If you have any doubts, please refer to http://semver.org/
#define MOM_CURRENT_VERSION "0.5.11"

#define MOM_COLORIZATION_CHECK_FREQUENCY 0.1f

//buffers for cstr variables
#define BUFSIZETIME (sizeof("00:00:00.000")+1)
#define BUFSIZELOCL (73)//Buffer size for localization/Max length for localized string
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


//Creates a convar, mainly used for MAKE_TOGGLE
#define MAKE_CONVAR(name, defaultval, flags, desc, minVal, maxVal)                                                            \
    ConVar name(#name, defaultval, flags, desc, true, minVal, true, maxVal)

//Creates a CONVAR with 0 as the minimum value, and 1 as the max value. Useful for toggle variables.
#define MAKE_TOGGLE_CONVAR(name, defaultval, flags, desc) MAKE_CONVAR(name, defaultval, flags, desc, 0, 1)

//Flags for a HUD cvar (usually)
#define FLAG_HUD_CVAR (FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_REPLICATED)

#define MAX_STAGES 64

// I'm a deadbeat, so I did this to stop having to worry about what MOM_APIDOMAIN is
// Set this macro to 0 to use momentum-mod.org as the webdomain, otherwise it uses the local domain (Or whatever you set)
// Make sure this is 0 when you push!
#define MOM_USINGLOCALWEB 1

#if MOM_USINGLOCALWEB
// What is the URL of the web?
#define MOM_WEBDOMAIN "http://127.0.0.1:5000"
#else
#define MOM_WEBDOMAIN "https://momentum-mod.org"
#endif

// Where to query the api. In case it does not match the current WEBDOMAIN (How did you end up like this?), you can change it!
#define MOM_APIDOMAIN MOM_WEBDOMAIN

#define MAP_FOLDER "maps" //MOM_TODO: Ensure all files are successfully built using V_ComposeFile
#define RECORDING_PATH "recordings"
#define EXT_TIME_FILE ".tim"
#define EXT_ZONE_FILE ".zon"
#define EXT_RECORDING_FILE ".momrec"

#endif // MOM_SHAREDDEFS_H