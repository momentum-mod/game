#include "cbase.h"
#include "mom_replay_entity.h"
#include "util/mom_util.h"

static ConVar mom_replay_firstperson("mom_replay_firstperson", "0", 
    FCVAR_CLIENTCMD_CAN_EXECUTE, "Watch replay in first-person", true, 0, true, 1);
static ConVar mom_replay_ghost_bodygroup("mom_replay_ghost_bodygroup", "11", 
    FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Replay ghost's body group (model)", true, 0, true, 14);
static ConCommand mom_replay_ghost_color("mom_replay_ghost_color",
    CMomentumReplayGhostEntity::SetGhostColor, "Set the ghost's color. Accepts HEX color value in format RRGGBBAA", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE);
static ConVar mom_replay_ghost_alpha("mom_replay_ghost_alpha", "75",
    FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Sets the ghost's transparency, integer between 0 and 255,", true, 0, true, 255);

LINK_ENTITY_TO_CLASS(mom_replay_ghost, CMomentumReplayGhostEntity);

BEGIN_DATADESC(CMomentumReplayGhostEntity)
END_DATADESC()

Color CMomentumReplayGhostEntity::m_newGhostColor = COLOR_GREEN;

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
    SetBodygroup(1, mom_replay_ghost_bodygroup.GetInt());
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
    SetNextThink(gpGlobals->curtime + gpGlobals->interval_per_tick);
}

void CMomentumReplayGhostEntity::HandleGhost() {
    if (!m_bIsActive) 
    {
        if (!Q_strcmp(m_pszMapName, STRING(gpGlobals->mapname)) == 0) {
			DispatchSpawn(this);
        }
	}
	else 
    {
        if (currentStep.m_vPlayerOrigin.x == 0.0f)
            return;

        if (mom_replay_firstperson.GetBool())
        {
            CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
            if (pPlayer) {
                //MOM_TODO: interpolate eyeangles and origin somehow so playback doesn't look so jerky
                pPlayer->SnapEyeAngles(currentStep.m_qEyeAngles);
                pPlayer->SetAbsOrigin(currentStep.m_vPlayerOrigin);

                pPlayer->m_nButtons &= currentStep.m_nPlayerButtons; // MOM_TODO: make this actually work
                pPlayer->AddFlag(FL_ATCONTROLS); //prevent keypress from affecting the replay playback 
            }
        }
        else
        {
            SetAbsOrigin(currentStep.m_vPlayerOrigin);
            SetAbsAngles(QAngle(currentStep.m_qEyeAngles.x / 10, //we divide x angle (pitch) by 10 so the ghost doesn't look really stupid
                currentStep.m_qEyeAngles.y, currentStep.m_qEyeAngles.z)); 
        }
	}
    //update color, bodygroup, and other params if they change
    if (mom_replay_ghost_bodygroup.GetInt() != m_iBodyGroup)
    {
        m_iBodyGroup = mom_replay_ghost_bodygroup.GetInt();
        SetBodygroup(1, m_iBodyGroup);
    }
    if (m_ghostColor != m_newGhostColor)
    {
        m_ghostColor = m_newGhostColor;
        SetRenderColor(m_ghostColor.r(), m_ghostColor.g(), m_ghostColor.b());
    }
    if (mom_replay_ghost_alpha.GetInt() != m_ghostColor.a())
    {
        m_ghostColor.SetColor(m_ghostColor.r(), m_ghostColor.g(), m_ghostColor.b(), //we have to set the previous colors in order to change alpha...
            mom_replay_ghost_alpha.GetInt());
        SetRenderColorA(mom_replay_ghost_alpha.GetInt());
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
void CMomentumReplayGhostEntity::SetGhostColor(const CCommand &args)
{
    if (mom_UTIL.GetColorFromHex(args.ArgS()))
        CMomentumReplayGhostEntity::m_newGhostColor = *mom_UTIL.GetColorFromHex(args.ArgS());
}
void CMomentumReplayGhostEntity::EndRun()
{
	SetNextThink(-1);
	Remove();
    m_bIsActive = false;
    CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
    if (pPlayer) {
        pPlayer->EnableControl(true);
    }
}
void CMomentumReplayGhostEntity::clearRunData()
{
    g_ReplaySystem->m_vecRunData.RemoveAll();
}

