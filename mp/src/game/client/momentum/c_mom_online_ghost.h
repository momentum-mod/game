#pragma once

#include "cbase.h"
#include "c_mom_ghost_base.h"
#include "interpolatedvar.h"
#include "GhostEntityPanel.h"
#include "steam/steam_api.h"

class CGhostEntityPanel;

class C_MomentumOnlineGhostEntity : public C_MomentumGhostBaseEntity
{
    DECLARE_CLASS(C_MomentumOnlineGhostEntity, C_MomentumGhostBaseEntity);
    DECLARE_CLIENTCLASS();
    DECLARE_INTERPOLATION()

public:
    C_MomentumOnlineGhostEntity();
    ~C_MomentumOnlineGhostEntity();

    void Spawn(void) OVERRIDE;
    void ClientThink(void) OVERRIDE;

    bool IsOnlineGhost() const OVERRIDE { return true; }

    //recieved from CMomentumOnlineGhostEntity serverclass 
    char m_pszGhostName[MAX_PLAYER_NAME_LENGTH]; 
    uint32 m_uiAccountID;
    CSteamID m_SteamID;
    int m_nGhostButtons;
    bool m_bSpectating; /// Is this ghost currently spectating?


    bool m_bSpectated; // Is this ghost being spectated by us?

private:

    CGhostEntityPanel *m_pEntityPanel;
};