#pragma once

#include "cbase.h"
#include "mom_entity_run_data.h"

class C_MomentumReplayGhostEntity : public C_BaseAnimating
{
    DECLARE_CLASS(C_MomentumReplayGhostEntity, C_BaseAnimating);
    DECLARE_CLIENTCLASS();

public:
    C_MomentumReplayGhostEntity();

    CMOMRunEntityData m_RunData;

    int m_nReplayButtons;
    int m_iTotalStrafes;

};