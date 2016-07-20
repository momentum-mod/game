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

#define PANEL_TIMES "times"

// Main Version (0 is alpha, 1 is beta, 2 is release)​.Main feature push (increment by one for each)​.​Small commits or hotfixes​
// When editing this, remember to also edit version.txt on the main dir of the repo
// If you have any doubts, please refer to http://semver.org/
#define MOM_CURRENT_VERSION "0.5.0"

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
// Comment this next definition to use MOM_WEBDOMAIN as MOM_APIDOMAIN
// Make sure this is commented when you push!
//#define MOM_USINGLOCALAPI

// What is the URL of the web?
#define MOM_WEBDOMAIN "http://momentum-mod.org"

// Where to query the api. (here so we can change between live server and test adress  (127.0....) easier
// ensure that this equals MOM_WEBDOMAIN before pushing! (!!MOM_USINGLOCALAPI also has to be commented!!))
#ifndef MOM_USINGLOCALAPI
#define MOM_APIDOMAIN MOM_WEBDOMAIN
#else
// You can change this adress if you use some other url
#define MOM_APIDOMAIN "http://127.0.0.1:5000"
#endif

#define MAP_FOLDER "maps" //MOM_TODO: Ensure all files are successfully built using V_ComposeFile
#define RECORDING_PATH "recordings"
#define EXT_TIME_FILE ".tim" //MOM_TODO: Find and replace all instances, no hardcode.
#define EXT_ZONE_FILE ".zon" //MOM_TODO: Find and replace all instances, no hardcode.
#define EXT_RECORDING_FILE ".momrec"

#endif // MOM_SHAREDDEFS_H