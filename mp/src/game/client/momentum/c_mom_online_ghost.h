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

    void Spawn() OVERRIDE;
    void ClientThink() OVERRIDE;

    bool IsOnlineGhost() const OVERRIDE { return true; }

    bool m_bSpectating; /// Is this ghost currently spectating?

    RUN_ENT_TYPE GetEntType() OVERRIDE { return RUN_ENT_ONLINE; }

    void Simulate() OVERRIDE;
    void CreateLightEffects() OVERRIDE;

private:
    CFlashlightEffect *m_pFlashlight;
    CGhostEntityPanel *m_pEntityPanel;
};