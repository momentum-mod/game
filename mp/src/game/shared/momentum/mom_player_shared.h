#pragma once

#ifdef CLIENT_DLL
#include "momentum/c_mom_player.h"
#define CMomentumPlayer C_MomentumPlayer
#else
#include "momentum/mom_player.h"
#endif

inline CMomentumPlayer *ToCMOMPlayer(CBaseEntity *pEntity)
{
    if (!pEntity || !pEntity->IsPlayer())
        return nullptr;
    return dynamic_cast<CMomentumPlayer*>(pEntity);
}