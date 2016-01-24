#ifndef MOM_SHAREDDEFS_H
#define MOM_SHAREDDEFS_H
#ifdef _WIN32
#pragma once
#endif


#include "const.h"
#include "shareddefs.h"

// naming this enum would be helpfull
typedef enum
{
    MOMGM_UNKNOWN = 0,
    MOMGM_SURF,
    MOMGM_BHOP,
    MOMGM_SCROLL,
    
} GAMEMODES;
#endif // MOM_SHAREDDEFS_H