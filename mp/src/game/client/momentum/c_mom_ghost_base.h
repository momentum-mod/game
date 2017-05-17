#pragma once
#include "cbase.h"

class C_MomentumGhostBaseEntity : public C_BaseAnimating
{
    DECLARE_CLASS(C_MomentumGhostBaseEntity, C_BaseAnimating);
    DECLARE_CLIENTCLASS();
    DECLARE_INTERPOLATION()
public:
    
    virtual bool IsReplayGhost() const { return false; }
    virtual bool IsOnlineGhost() const { return false; }

protected:
    bool ShouldInterpolate() OVERRIDE { return true; }

};