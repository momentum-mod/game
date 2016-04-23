#include "cbase.h"
#include "mom_replay_entity.h"


#define MODEL "models/cone.mdl"

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
    m_ghostColor = COLOR_BLUE; //default color
}

//-----------------------------------------------------------------------------
// Purpose: Sets up the entity's initial state
//-----------------------------------------------------------------------------
void CMomentumReplayGhostEntity::Spawn(void)
{
	Precache();
	RemoveEffects(EF_NODRAW);
	SetModel(MODEL);
	SetSolid( SOLID_NONE );
	SetRenderMode(kRenderTransColor);
    SetRenderColor(m_ghostColor.r(), m_ghostColor.g(), m_ghostColor.b());
	SetRenderColorA(75);
	SetMoveType( MOVETYPE_NOCLIP );
    m_bIsActive = true;
}

void CMomentumReplayGhostEntity::StartRun() 
{
	//Msg("Starting run with Rundata: %i, Step %i, Name %s, Starttime: %f, This: %i\n", RunData.size(), step, m_gName, startTime, this);
    m_nStartTick = gpGlobals->curtime;
    m_bIsActive = true;
    step = 0;
    SetAbsOrigin(m_entRunData[0]->m_vPlayerOrigin);

	SetNextThink(gpGlobals->curtime);
}
void CMomentumReplayGhostEntity::updateStep() 
{
    const size_t numTicks = m_entRunData.Count();
    if (step < 0 || step >= numTicks)
    {
		currentStep = nextStep = NULL;
		return;
	}
    currentStep = m_entRunData[step];
    int currentTick = gpGlobals->tickcount - m_nStartTick;

    //catching up to a fast ghost, you came in late
    if (currentTick > currentStep->m_nCurrentTick)
    {
		unsigned int x = step + 1;
        while (++x < numTicks)
        {
            if (currentTick < m_entRunData[x]->m_nCurrentTick) {
				break;
			}
		}
		step = x - 1;
	}
    currentStep = m_entRunData[step];//update it to the new step
    currentTick = gpGlobals->tickcount - m_nStartTick;//update to new time

    if (step == (numTicks - 1)) //if it's on the last step
    {
        //end the run somehow
	} 
    else 
    {
        nextStep = m_entRunData[step + 1];
	}
}
//-----------------------------------------------------------------------------
// Purpose: Think function to move the ghost
//-----------------------------------------------------------------------------
void CMomentumReplayGhostEntity::Think(void)
{
	BaseClass::Think();
    if (Q_strlen(m_pszPlayerName) != 0)
    {
		if (!IsEffectActive(EF_NODRAW)) 
            EntityText(0, m_pszPlayerName, 0);
		updateStep();
		HandleGhost();
	} 
    else 
		EndRun();

	SetNextThink(gpGlobals->curtime + gpGlobals->interval_per_tick);
}

void CMomentumReplayGhostEntity::HandleGhost() {
	if (currentStep != NULL) 
    {
        if (!m_bIsActive)
        {
			if (!Q_strcmp(m_pszMapName, STRING(gpGlobals->mapname)) == 0) 
				DispatchSpawn(this);
		} 
		else 
        {
            float x = currentStep->m_vPlayerOrigin.x;
            float y = currentStep->m_vPlayerOrigin.y;
            float z = currentStep->m_vPlayerOrigin.z;
            float angleX = currentStep->m_qEyeAngles.x;
            float angleY = currentStep->m_qEyeAngles.y;
            float angleZ = currentStep->m_qEyeAngles.z;
            int t1 = currentStep->m_nCurrentTick;

            if (x == 0.0f)
                return;

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
                int t2 = nextStep->m_nCurrentTick;

                //interpolate position
				float scalar = (((gpGlobals->tickcount - m_nStartTick) - t1) / (t2 - t1)); //time difference scalar value used to interpolate

				float xfinal = x + (scalar * (x2 - x));
				float yfinal = y + (scalar * (y2 - y));
				float zfinal = z + (scalar * (z2 - z));
				SetAbsOrigin(Vector(xfinal, yfinal, (zfinal - 15.0f))); //@Tuxxi: @gocnak, why are we subtracting 15.0 here?
                float angleXFinal = angleX + (scalar * (angleX2 - angleX));
                float angleYFinal = angleY + (scalar * (angleY2 - angleY));
                float angleZFinal = angleZ + (scalar * (angleZ2 - angleZ));
                SetAbsAngles(QAngle(angleXFinal, angleYFinal, angleZFinal));
			}
            else //we cant interpolate
            {
                SetAbsOrigin(Vector(x, y, (z - 15.0f)));
                SetAbsAngles(QAngle(angleX, angleY, angleZ));
            }
		}
	}
    else
    {
        // END RUN
    }
}

void CMomentumReplayGhostEntity::SetGhostModel(const char * newmodel)
{
	if (newmodel) {
        Q_strcpy(m_pszModel, newmodel);
        PrecacheModel(m_pszModel);
        SetModel(m_pszModel);
	}
}
void CMomentumReplayGhostEntity::EndRun()
{
	SetNextThink(0.0f);
	Remove();
    m_bIsActive = false;
}
void CMomentumReplayGhostEntity::clearRunData()
{
    m_entRunData.RemoveAll();
}