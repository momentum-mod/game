#include "cbase.h"
#include "mom_replay_entity.h"
#include "util/mom_util.h"
#include "Timer.h"

static ConVar mom_replay_firstperson("mom_replay_firstperson", "0", 
    FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Watch replay in first-person", true, 0, true, 1);
static ConVar mom_replay_reverse("mom_replay_reverse", "0",
    FCVAR_CLIENTCMD_CAN_EXECUTE, "Reverse playback of replay", true, 0, true, 1);
static ConVar mom_replay_ghost_bodygroup("mom_replay_ghost_bodygroup", "11", 
    FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Replay ghost's body group (model)", true, 0, true, 14);
static ConCommand mom_replay_ghost_color("mom_replay_ghost_color",
    CMomentumReplayGhostEntity::SetGhostColor, "Set the ghost's color. Accepts HEX color value in format RRGGBB", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE);
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
	SetRenderMode(kRenderTransColor);
    SetRenderColor(m_ghostColor.r(), m_ghostColor.g(), m_ghostColor.b(), 75);
	SetMoveType(MOVETYPE_NOCLIP);
    RemoveSolidFlags(FSOLID_NOT_SOLID);
    AddSolidFlags(FSOLID_TRIGGER);
    SetModel(GHOST_MODEL);
    SetBodygroup(1, mom_replay_ghost_bodygroup.GetInt());
}

void CMomentumReplayGhostEntity::StartRun() 
{
    Spawn();
    m_nStartTick = gpGlobals->curtime;
    m_bIsActive = true;
    step = 1;
    SetAbsOrigin(g_ReplaySystem->m_vecRunData[0].m_vPlayerOrigin);
	SetNextThink(gpGlobals->curtime);
}
void CMomentumReplayGhostEntity::updateStep() 
{
    currentStep = g_ReplaySystem->m_vecRunData[step];

    if (mom_replay_reverse.GetBool())
    {
        nextStep = g_ReplaySystem->m_vecRunData[--step];
    }
    else
    {
        nextStep = g_ReplaySystem->m_vecRunData[++step];
    }
}
//-----------------------------------------------------------------------------
// Purpose: Think function to move the ghost
//-----------------------------------------------------------------------------
void CMomentumReplayGhostEntity::Think(void)
{
	BaseClass::Think();
    if (step < g_ReplaySystem->m_vecRunData.Count() && step >= 1) 
    {
        updateStep();
        HandleGhost();
	}
    else
    {
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

        CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
        if (pPlayer)
        {
            if (mom_replay_firstperson.GetBool())
            {
                pPlayer->m_bIsWatchingReplay = true;
                if (!pPlayer->IsObserver())
                {
                    pPlayer->SetObserverTarget(this);
                    pPlayer->StartObserverMode(OBS_MODE_IN_EYE);
                }
                pPlayer->RemoveSolidFlags(FSOLID_NOT_SOLID);

                if (pPlayer->GetObserverMode() != (OBS_MODE_IN_EYE | OBS_MODE_CHASE)) {
                    //we don't want to allow any other obs modes, only IN EYE and CHASE
                    pPlayer->ForceObserverMode(OBS_MODE_IN_EYE);
                }
                if (pPlayer->GetObserverMode() == OBS_MODE_IN_EYE) {
                    //for some reason we can't change the view offset so we have to modify the origin when spectating in first person.
                    SetAbsOrigin(currentStep.m_vPlayerOrigin + VEC_VIEW);
                    SetAbsAngles(currentStep.m_qEyeAngles);
                }
                else {
                    SetAbsOrigin(currentStep.m_vPlayerOrigin);
                    SetAbsAngles(QAngle(currentStep.m_qEyeAngles.x / 10, //we divide x angle (pitch) by 10 so the ghost doesn't look really stupid
                        currentStep.m_qEyeAngles.y, currentStep.m_qEyeAngles.z));
                }

                //interpolate vel from difference in origin
                float distX = fabs(currentStep.m_vPlayerOrigin.x - nextStep.m_vPlayerOrigin.x);
                float distY= fabs(currentStep.m_vPlayerOrigin.y - nextStep.m_vPlayerOrigin.y);
                float distZ = fabs(currentStep.m_vPlayerOrigin.z - nextStep.m_vPlayerOrigin.z);
                Vector interpolatedVel = Vector(distX, distY, distZ) / gpGlobals->interval_per_tick;
                SetAbsVelocity(interpolatedVel);

                pPlayer->m_nReplayButtons = currentStep.m_nPlayerButtons; //networked var that allows the replay to control keypress display on the client

                if (g_Timer.IsRunning())
                    UpdateStats(interpolatedVel, pPlayer); 

                if (currentStep.m_nPlayerButtons & IN_DUCK)
                {
                    //MOM_TODO: make this smoother. possibly inherit from NPC classes/CBaseCombatCharacter
                    SetAbsOrigin(currentStep.m_vPlayerOrigin + VEC_DUCK_VIEW);
                }
            }
            else //we're watching/racing with a ghost
            {
                SetAbsOrigin(currentStep.m_vPlayerOrigin);
                SetAbsAngles(QAngle(currentStep.m_qEyeAngles.x / 10, //we divide x angle (pitch) by 10 so the ghost doesn't look really stupid
                    currentStep.m_qEyeAngles.y, currentStep.m_qEyeAngles.z));
                if (pPlayer->IsObserver()) //bring the player out of obs mode if theyre currently observing
                {
                    pPlayer->m_bIsWatchingReplay = false;
                    pPlayer->StopObserverMode();
                    pPlayer->ForceRespawn();
                    pPlayer->SetMoveType(MOVETYPE_WALK);
                }
            }
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
void CMomentumReplayGhostEntity::UpdateStats(Vector ghostVel, CMomentumPlayer *pPlayer)
{
    // --- STRAFE SYNC ---
    //calculate strafe sync based on replay ghost's movement, in order to update the player's HUD

    float SyncVelocity = ghostVel.Length2DSqr(); //we always want HVEL for checking velocity sync
    if (!(pPlayer->GetFlags() & (FL_ONGROUND | FL_INWATER)) && pPlayer->GetMoveType() != MOVETYPE_LADDER)
    {
        if (EyeAngles().y > m_qLastEyeAngle.y) //player turned left 
        {
            m_nStrafeTicks++;
            if ((currentStep.m_nPlayerButtons & IN_MOVELEFT) && !(currentStep.m_nPlayerButtons & IN_MOVERIGHT))
                m_nPerfectSyncTicks++;
            if (SyncVelocity > m_flLastSyncVelocity)
                m_nAccelTicks++;
        }
        else if (EyeAngles().y < m_qLastEyeAngle.y) //player turned right 
        {
            m_nStrafeTicks++;
            if ((currentStep.m_nPlayerButtons & IN_MOVERIGHT) && !(currentStep.m_nPlayerButtons & IN_MOVELEFT))
                m_nPerfectSyncTicks++;
            if (SyncVelocity > m_flLastSyncVelocity)
                m_nAccelTicks++;
        }
    }
    if (m_nStrafeTicks && m_nAccelTicks && m_nPerfectSyncTicks)
    {
        pPlayer->m_flStrafeSync = ((float)m_nPerfectSyncTicks / (float)m_nStrafeTicks) * 100; // ticks strafing perfectly / ticks strafing
        pPlayer->m_flStrafeSync2 = ((float)m_nAccelTicks / (float)m_nStrafeTicks) * 100; // ticks gaining speed / ticks strafing
    }
    // --- JUMP AND STRAFE COUNTER ---
    if (ghostVel.z == 0 && currentStep.m_nPlayerButtons & IN_JUMP)
        pPlayer->m_nStageJumps[0]++;
    if ((currentStep.m_nPlayerButtons & IN_MOVELEFT && !(m_nOldReplayButtons & IN_MOVELEFT)) 
        || (currentStep.m_nPlayerButtons & IN_MOVERIGHT && !(m_nOldReplayButtons & IN_MOVERIGHT)) )
        pPlayer->m_nStageStrafes[0]++;

    m_flLastSyncVelocity = SyncVelocity;
    m_qLastEyeAngle = EyeAngles();
    m_nOldReplayButtons = currentStep.m_nPlayerButtons;
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
    g_Timer.Stop(false);
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());

    if (pPlayer && pPlayer->IsObserver())
    {
        pPlayer->StopObserverMode();
        pPlayer->ForceRespawn();
        pPlayer->SetMoveType(MOVETYPE_WALK);
        pPlayer->m_bIsWatchingReplay = false;
        pPlayer->m_flLastJumpVel = 0;
    }
}