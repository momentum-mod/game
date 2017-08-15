#pragma once

#include "cbase.h"
#include "c_mom_online_ghost.h"
#include "vgui_entitypanel.h"
#include "vgui_avatarimage.h"

class C_MomentumOnlineGhostEntity;

class CGhostEntityPanel : public CEntityPanel
{
    DECLARE_CLASS_SIMPLE(CGhostEntityPanel, CEntityPanel);

    CGhostEntityPanel();
    ~CGhostEntityPanel();

    void Init(C_MomentumOnlineGhostEntity *pEntity);

private:

    C_MomentumOnlineGhostEntity *m_pEntity;
    CAvatarImagePanel *m_pAvatarImage;
};
