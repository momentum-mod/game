#pragma once

class C_MomentumGhostBaseEntity : public C_BaseAnimating
{
    DECLARE_CLASS(C_MomentumGhostBaseEntity, C_BaseAnimating);
    DECLARE_CLIENTCLASS();
    DECLARE_INTERPOLATION()
public:

    C_MomentumGhostBaseEntity();

    bool IsValidIDTarget(void) OVERRIDE{ return true; }

    virtual bool IsReplayGhost() const { return false; }
    virtual bool IsOnlineGhost() const { return false; }

    CNetworkVar(int, m_iDisabledButtons);
    CNetworkVar(bool, m_bBhopDisabled);

    CInterpolatedVar<Vector> m_iv_vecViewOffset;

protected:
    bool ShouldInterpolate() OVERRIDE { return true; }

};