#ifndef GHOST_BASE
#define GHOST_BASE 
#pragma once

#include "cbase.h"
#include "mom_timer.h"

#define GHOST_MODEL "models/player/player_shape_base.mdl"

enum ghostModelBodyGroup
{
    BODY_THREE_SIDED_PYRAMID = 0,
    BODY_FOUR_SIDED_PYRAMID,
    BODY_SIX_SIDED_PYRAMID,
    BODY_CUBE,
    BODY_FOUR_SIDED_PRISM,
    BODY_THREE_SIDED_PRISM,
    BODY_KITE,
    BODY_FIVE_SIDED_PRISM,
    BODY_SIX_SIDED_PRISM,
    BODY_PENTAGON_BALL,
    BODY_BALL,
    BODY_PROLATE_ELLIPSE,
    BODY_TRIANGLE_BALL,
    BODY_CONE,
    BODY_CYLINDER,
    LAST
};
class CMomentumPlayer;
class CMomentumGhostBaseEntity : public CBaseAnimating
{
    DECLARE_CLASS(CMomentumGhostBaseEntity, CBaseAnimating);
    DECLARE_DATADESC();
    DECLARE_SERVERCLASS();

public:
    CMomentumGhostBaseEntity();

    virtual void SetGhostBodyGroup(int bodyGroup);
    virtual void SetGhostColor(const uint32 newHexColor);
    virtual void SetGhostTrailProperties(const uint32 newHexColor, int newLen, bool enable);
    virtual void SetGhostFlashlight(bool isOn);

    virtual void SetGhostAppearance(ghostAppearance_t app, bool bForceUpdate = false);
    virtual ghostAppearance_t GetAppearance() { return m_ghostAppearance; }

    virtual void StartTimer(int m_iStartTick);
    virtual void StopTimer();

    //Pure virtual functions
    virtual void HandleGhost() = 0;
    virtual void HandleGhostFirstPerson() = 0;
    virtual void UpdateStats(const Vector &ghostVel) = 0; // for hud display..

    virtual bool IsReplayGhost() const { return false; }
    virtual bool IsOnlineGhost() const { return false; }

    void SetSpectator(CMomentumPlayer *player) { m_pCurrentSpecPlayer = player; }
    void RemoveSpectator()
    {
        m_pCurrentSpecPlayer = nullptr;
        UnHideGhost();
    }
    CMomentumPlayer* GetCurrentSpectator() { return m_pCurrentSpecPlayer; }


    void HideGhost()
    {
        // don't render the model when we're in first person mode
        if (GetRenderMode() != kRenderNone)
        {
            SetRenderMode(kRenderNone);
            AddEffects(EF_NOSHADOW);
        }
    }
    void UnHideGhost()
    {
        if (GetRenderMode() != kRenderTransColor)
        {
            SetRenderMode(kRenderTransColor);
            RemoveEffects(EF_NOSHADOW);
        }
    }

protected:
    virtual void Think(void);
    virtual void Spawn(void);
    virtual void Precache(void);
    void DecalTrace(trace_t* pTrace, char const* decalName) OVERRIDE {} // Don't do any DecalTracing on this entity
    int UpdateTransmitState() OVERRIDE { return SetTransmitState(FL_EDICT_ALWAYS); }
    int ShouldTransmit(const CCheckTransmitInfo* pInfo) OVERRIDE { return FL_EDICT_ALWAYS; }

    virtual void CreateTrail();
    virtual void RemoveTrail();

    bool CanUnduck(CMomentumGhostBaseEntity *pGhost);
    CMomentumPlayer *m_pCurrentSpecPlayer;
    ghostAppearance_t m_ghostAppearance;

private:
    CBaseEntity *m_eTrail;
};
#endif //GHOST_BASE