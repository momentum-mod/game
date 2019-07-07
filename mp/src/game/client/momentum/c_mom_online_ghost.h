#pragma once

#include "c_mom_ghost_base.h"

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
    uint32 m_uiAccountID;
    uint64 m_SteamID;
    bool m_bSpectating; /// Is this ghost currently spectating?

    RUN_ENT_TYPE GetEntType() OVERRIDE { return RUN_ENT_ONLINE; }

    void Simulate() OVERRIDE;
    void CreateLightEffects() OVERRIDE;

private:
    CFlashlightEffect *m_pFlashlight;
    CGhostEntityPanel *m_pEntityPanel;
};