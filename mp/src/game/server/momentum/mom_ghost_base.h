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

    virtual void SetGhostModel(const char *model);
    virtual void SetGhostBodyGroup(int bodyGroup);
    virtual void SetGhostColor(const uint32 newHexColor);
    virtual void SetGhostTrailProperties(const uint32 newHexColor, int newLen, bool enable);

    virtual void SetGhostAppearance(ghostAppearance_t app);
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
    void RemoveSpectator() { m_pCurrentSpecPlayer = nullptr; }
    CMomentumPlayer* GetCurrentSpectator() { return m_pCurrentSpecPlayer; }

protected:
    virtual void Think(void);
    virtual void Spawn(void);
    virtual void Precache(void);

    virtual void CreateTrail();
    virtual void RemoveTrail();

    bool CanUnduck(CMomentumGhostBaseEntity *pGhost);
    CMomentumPlayer *m_pCurrentSpecPlayer;
    ghostAppearance_t m_ghostAppearance;

private:
    bool trailEnable;
    CBaseEntity *m_eTrail;

    bool hasSpawned;
    bool hasSetAppearance;
};
#endif //GHOST_BASE