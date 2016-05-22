#ifndef MOM_REPLAY_GHOST_H
#define MOM_REPLAY_GHOST_H

#include "cbase.h"
#include "in_buttons.h"
#include "replayformat.h"
#include "mom_entity_run_data.h"

#pragma once

#define GHOST_MODEL "models/player/player_shape_base.mdl"
#define ALL_BUTTONS IN_LEFT & IN_RIGHT & IN_MOVELEFT & IN_MOVERIGHT & IN_FORWARD & IN_BACK & IN_JUMP & IN_DUCK
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
    BODY_CYLINDER
};

class CMomentumReplayGhostEntity : public CBaseAnimating
{
    DECLARE_CLASS(CMomentumReplayGhostEntity, CBaseAnimating);
	DECLARE_DATADESC();
    DECLARE_SERVERCLASS();
public:
    CMomentumReplayGhostEntity();
    ~CMomentumReplayGhostEntity();
	const char* GetGhostModel() const;
	void SetGhostModel(const char* model);
    void SetGhostBodyGroup(int bodyGroup);
    static void SetGhostColor(const CCommand &args);
	//Increments the steps intelligently.
	void UpdateStep();

	void EndRun();
	void StartRun(bool firstPerson = false, bool shouldLoop = false);
	void HandleGhost();
    void HandleGhostFirstPerson();
    void UpdateStats(Vector ghostVel); //for hud display..

    bool m_bIsActive;
    int m_nStartTick;

    CNetworkVarEmbedded(CMOMRunEntityData, m_RunData);
    CNetworkVar(int, m_nReplayButtons);
    CNetworkVar(int, m_iTotalStrafes);
    CNetworkVar(int, m_iTotalJumps);

protected:
	void Think(void) override;
    void Spawn(void) override;
    void Precache(void) override;

private:
    char m_pszModel[256], m_pszMapName[256];
	replay_frame_t currentStep; 
    replay_frame_t nextStep;	

    //MOM_TODO: CUtlVector<CMomentumPlayer*> spectators;

    int step;
    int m_iBodyGroup = BODY_PROLATE_ELLIPSE;
    Color m_ghostColor;
    static Color m_newGhostColor;
    bool m_bHasJumped;
    //for faking strafe sync calculations
    QAngle m_qLastEyeAngle;
    float m_flLastSyncVelocity;
    int m_nStrafeTicks, m_nPerfectSyncTicks, m_nAccelTicks, m_nOldReplayButtons;
    bool m_bReplayShouldLoop;
};

#endif // MOM_REPLAY_GHOST_H
