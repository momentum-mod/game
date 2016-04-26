#include "cbase.h"
#include "mom_replay_entity.h"

LINK_ENTITY_TO_CLASS(mom_replay_ghost, CMomentumReplayGhostEntity);

BEGIN_DATADESC(CMomentumReplayGhostEntity)
END_DATADESC()

const char* CMomentumReplayGhostEntity::GetGhostModel() 
{
	return m_pszModel;
}

void CMomentumReplayGhostEntity::Precache(void)
{
	BaseClass::Precache();
    PrecacheModel(GHOST_MODEL);
    m_ghostColor = COLOR_GREEN; //default color
}

//-----------------------------------------------------------------------------
// Purpose: Sets up the entity's initial state
//-----------------------------------------------------------------------------
void CMomentumReplayGhostEntity::Spawn(void)
{
    BaseClass::Spawn();
	Precache();
	RemoveEffects(EF_NODRAW);
	SetSolid(SOLID_NONE);
	SetRenderMode(kRenderTransColor);
    SetRenderColor(m_ghostColor.r(), m_ghostColor.g(), m_ghostColor.b());
	SetRenderColorA(75);
	SetMoveType(MOVETYPE_NOCLIP);
    m_bIsActive = true; 
    SetModel(GHOST_MODEL);
    SetBodygroup(1, m_iBodyGroup);
}

void CMomentumReplayGhostEntity::StartRun() 
{
    Spawn();
	//Msg("Starting run with Rundata: %i, Step %i, Name %s, Starttime: %f, This: %i\n", RunData.size(), step, m_gName, startTime, this);
    m_nStartTick = gpGlobals->curtime;
    m_bIsActive = true;
    step = 1;
    SetAbsOrigin(g_ReplaySystem->m_vecRunData[0].m_vPlayerOrigin);

	SetNextThink(gpGlobals->curtime);
}
void CMomentumReplayGhostEntity::updateStep() 
{
    currentStep = g_ReplaySystem->m_vecRunData[step];
    step++;
}
//-----------------------------------------------------------------------------
// Purpose: Think function to move the ghost
//-----------------------------------------------------------------------------
void CMomentumReplayGhostEntity::Think(void)
{
	BaseClass::Think();
    if (step < g_ReplaySystem->m_vecRunData.Count()) {
        updateStep();
        HandleGhost();
	} 
    else {
        EndRun();
    }
    DevLog("Ghost X: %f Y: %f Z: %f\n", 
        currentStep.m_vPlayerOrigin.x, currentStep.m_vPlayerOrigin.y, currentStep.m_vPlayerOrigin.z);
	SetNextThink(gpGlobals->curtime + gpGlobals->interval_per_tick);
}

void CMomentumReplayGhostEntity::HandleGhost() {
    if (!m_bIsActive) {
        if (!Q_strcmp(m_pszMapName, STRING(gpGlobals->mapname)) == 0) {
			DispatchSpawn(this);
        }
	} 
	else 
    {
        float x = currentStep.m_vPlayerOrigin.x;
        float y = currentStep.m_vPlayerOrigin.y;
        float z = currentStep.m_vPlayerOrigin.z;
        float angleX = currentStep.m_qEyeAngles.x;
        float angleY = currentStep.m_qEyeAngles.y;
        float angleZ = currentStep.m_qEyeAngles.z;

        if (x == 0.0f)
            return;
        /*
		if (nextStep != NULL) // we have to be at least 2 ticks into the replay to interpolate
        {
			if (IsEffectActive(EF_NODRAW)) 
                RemoveEffects(EF_NODRAW);

            float x2 = nextStep->m_vPlayerOrigin.x;
            float y2 = nextStep->m_vPlayerOrigin.y;
            float z2 = nextStep->m_vPlayerOrigin.z;
            float angleX2 = nextStep->m_qEyeAngles.x;
            float angleY2 = nextStep->m_qEyeAngles.y;
            float angleZ2 = nextStep->m_qEyeAngles.z;

            //interpolate position
			float scalar = (((gpGlobals->tickcount - m_nStartTick) - t1) / (t2 - t1)); //time difference scalar value used to interpolate

			float xfinal = x + (scalar * (x2 - x));
			float yfinal = y + (scalar * (y2 - y));
			float zfinal = z + (scalar * (z2 - z));
			SetAbsOrigin(Vector(xfinal, yfinal, (zfinal - 15.0f))); //@tuxxi: @Gocnak, why are we subtracting 15.0 here?
            float angleXFinal = angleX + (scalar * (angleX2 - angleX));
            float angleYFinal = angleY + (scalar * (angleY2 - angleY));
            float angleZFinal = angleZ + (scalar * (angleZ2 - angleZ));
            SetAbsAngles(QAngle(angleXFinal, angleYFinal, angleZFinal));
		}
        else { //we cant interpolate 
        }
        */
        SetAbsOrigin(Vector(x, y, z));
        SetAbsAngles(QAngle(angleX, angleY, angleZ));
	}
}

void CMomentumReplayGhostEntity::SetGhostModel(const char * newmodel)
{
	if (newmodel) 
    {
        Q_strcpy(m_pszModel, newmodel);
        PrecacheModel(m_pszModel);
        SetModel(m_pszModel);
	}
}
void CMomentumReplayGhostEntity::SetGhostBodyGroup(int bodyGroup)
{
    if (bodyGroup > sizeof(ghostModelBodyGroup) || bodyGroup < 0) 
    {
        Msg("Error: Could not set bodygroup!");
        return;
    }
    else
    {
        m_iBodyGroup = bodyGroup;
        SetBodygroup(1, bodyGroup);
    }
}
void CMomentumReplayGhostEntity::SetGhostColor(Color newColor, int alpha)
{
    SetRenderColor(newColor.r(), newColor.g(), newColor.b());
    SetRenderColorA(alpha);
}
void CMomentumReplayGhostEntity::EndRun()
{
	SetNextThink(-1);
	Remove();
    m_bIsActive = false;
}
void CMomentumReplayGhostEntity::clearRunData()
{
    g_ReplaySystem->m_vecRunData.RemoveAll();
}