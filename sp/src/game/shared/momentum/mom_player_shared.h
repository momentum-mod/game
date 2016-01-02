#include "cbase.h"

#ifdef CLIENT_DLL
#include "momentum/c_mom_player.h"
#define CMomentumPlayer C_MomentumPlayer
#else
#include "momentum/mom_player.h"
#endif