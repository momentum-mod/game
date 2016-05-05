#include "cbase.h"
#include "mom_replay.h"
#include "in_buttons.h"

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
public:
	const char* GetGhostModel();
	void SetGhostModel(const char* model);
    void SetGhostBodyGroup(int bodyGroup);
    static void SetGhostColor(const CCommand &args);
	//Increments the steps intelligently.
	void updateStep();

	void EndRun();
	void StartRun();
	void HandleGhost();
    void UpdateStats(Vector ghostVel, CMomentumPlayer *pPlayer); //for hud display..

    bool m_bIsActive;
    int m_nStartTick;

protected:
	void Think(void) override;
    void Spawn(void) override;
    void Precache(void) override;

private:
    char m_pszModel[256], m_pszMapName[256];
	replay_frame_t currentStep; 
    replay_frame_t nextStep;	

    int step;
    int m_iBodyGroup = BODY_PROLATE_ELLIPSE;
    Color m_ghostColor;
    static Color m_newGhostColor;

    //for faking strafe sync calculations
    QAngle m_qLastEyeAngle;
    float m_flLastSyncVelocity;
    int m_nStrafeTicks, m_nPerfectSyncTicks, m_nAccelTicks, m_nOldReplayButtons;
};
