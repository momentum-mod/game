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
#define MOM_CURRENT_VERSION "0.2.5"

#define MOM_COLORIZATION_CHECK_FREQUENCY 0.1f

//buffers for cstr variables
#define BUFSIZETIME (sizeof("00:00:00.000")+1)
#define BUFSIZELOCL (73)
#define BUFSIZESHORT 10

#define MAX_STAGES 64

#endif // MOM_SHAREDDEFS_H