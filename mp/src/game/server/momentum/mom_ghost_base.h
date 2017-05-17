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
    //CMomentumGhostBaseEntity();
    //~CMomentumGhostBaseEntity();


    virtual void SetGhostModel(const char *model);
    virtual void SetGhostBodyGroup(int bodyGroup);
    virtual void SetGhostColor(const Color newColor) { m_NewGhostColor = newColor; }

    static void SetGhostColorCC(const CCommand &args);

    virtual void StartTimer(int m_iStartTick);
    virtual void StopTimer();

    //Pure virtual functions
    virtual void HandleGhost() = 0;
    virtual void HandleGhostFirstPerson() = 0;
    virtual void UpdateStats(const Vector &ghostVel) = 0; // for hud display..

    virtual bool IsReplayGhost() const { return false; }
    virtual bool IsOnlineGhost() const { return false; }

    const char *GetGhostModel() const { return m_pszModel; }
    int GetGhostBodyGroup() const { return m_iBodyGroup; }

    void SetSpectator(CMomentumPlayer *player) { m_pCurrentSpecPlayer = player; }
    void RemoveSpectator() { m_pCurrentSpecPlayer = nullptr; }
    CMomentumPlayer* GetCurrentSpectator() { return m_pCurrentSpecPlayer; }

protected:
    virtual void Think(void);
    virtual void Spawn(void);
    virtual void Precache(void);
    bool CanUnduck(CMomentumGhostBaseEntity *pGhost);
    CMomentumPlayer *m_pCurrentSpecPlayer;

private:
    int m_iBodyGroup;
    Color m_GhostColor;
    static Color m_NewGhostColor;
    char m_pszModel[256];
};
#endif //GHOST_BASE