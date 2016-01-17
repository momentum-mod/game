#ifndef MOM_SHAREDDEFS_H
#define MOM_SHAREDDEFS_H
#ifdef _WIN32
#pragma once
#endif


#include "const.h"
#include "shareddefs.h"

// naming this enum would be helpfull
enum
{
    MOMGM_BHOP = 0,
    MOMGM_SURF,
    MOMGM_SCROLL,
    MOMGM_UNKNOWN
};
#endif // MOM_SHAREDDEFS_H