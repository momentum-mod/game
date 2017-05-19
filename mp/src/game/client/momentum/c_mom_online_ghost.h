#pragma once

#include "cbase.h"
#include "c_mom_ghost_base.h"
#include "interpolatedvar.h"

class C_MomentumOnlineGhostEntity : public C_MomentumGhostBaseEntity
{
    DECLARE_CLASS(C_MomentumOnlineGhostEntity, C_MomentumGhostBaseEntity);
    DECLARE_CLIENTCLASS();
    DECLARE_INTERPOLATION()

public:
    C_MomentumOnlineGhostEntity();

    void Spawn(void) OVERRIDE;
    void ClientThink(void) OVERRIDE;

    bool IsOnlineGhost() const OVERRIDE { return true; }

    //recieved from CMomentumOnlineGhostEntity serverclass 
    char m_pszGhostName[MAX_PLAYER_NAME_LENGTH]; 

private:
};