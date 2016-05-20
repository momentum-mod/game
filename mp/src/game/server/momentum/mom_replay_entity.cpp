#include "cbase.h"
#include "mom_replay_entity.h"
#include "util/mom_util.h"
#include "Timer.h"
#include "mom_replay.h"
#include "mom_shareddefs.h"

MAKE_TOGGLE_CONVAR(mom_replay_firstperson, "1", FCVAR_CLIENTCMD_CAN_EXECUTE, "Watch replay in first-person");
MAKE_TOGGLE_CONVAR(mom_replay_reverse, "0", FCVAR_CLIENTCMD_CAN_EXECUTE, "Reverse playback of replay");
MAKE_TOGGLE_CONVAR(mom_replay_loop, "1", FCVAR_CLIENTCMD_CAN_EXECUTE, "Loop playback of replay ghost");
static ConVar mom_replay_ghost_bodygroup("mom_replay_ghost_bodygroup", "11", 
    FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Replay ghost's body group (model)", true, 0, true, 14);
static ConCommand mom_replay_ghost_color("mom_replay_ghost_color",
    CMomentumReplayGhostEntity::SetGhostColor, "Set the ghost's color. Accepts HEX color value in format RRGGBB", 
    FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE);
static ConVar mom_replay_ghost_alpha("mom_replay_ghost_alpha", "75",
    FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Sets the ghost's transparency, integer between 0 and 255,", true, 0, true, 255);

LINK_ENTITY_TO_CLASS(mom_replay_ghost, CMomentumReplayGhostEntity);

IMPLEMENT_SERVERCLASS_ST(CMomentumReplayGhostEntity, DT_MOM_ReplayEnt)
//MOM_TODO: Network other variables that the UI will need to reference
SendPropInt(SENDINFO(m_nReplayButtons)),
SendPropInt(SENDINFO(m_iTotalStrafes)),
SendPropDataTable(SENDINFO_DT(m_RunData), &REFERENCE_SEND_TABLE(DT_MOM_RunEntData)),
END_SEND_TABLE()

BEGIN_DATADESC(CMomentumReplayGhostEntity)
END_DATADESC()

Color CMomentumReplayGhostEntity::m_newGhostColor = COLOR_GREEN;

CMomentumReplayGhostEntity::CMomentumReplayGhostEntity()
{
    m_nReplayButtons = 0;
    m_iTotalStrafes = 0;
}


const char* CMomentumReplayGhostEntity::GetGhostModel() const
{
	return m_pszModel;
}
CMomentumReplayGhostEntity::~CMomentumReplayGhostEntity()
{
    g_ReplaySystem->m_bIsWatchingReplay = false;
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
    SetModel(GHOST_MODEL);
    SetBodygroup(1, mom_replay_ghost_bodygroup.GetInt());
}

void CMomentumReplayGhostEntity::StartRun(bool firstPerson, bool shouldLoop) 
{
    mom_replay_firstperson.SetValue(firstPerson ? "1" : "0");
    mom_replay_loop.SetValue(shouldLoop ? "1" : "0");

    Spawn();
    m_iTotalStrafes = 0;
    m_nStartTick = gpGlobals->curtime;
    m_bIsActive = true;
    step = 0;
    SetAbsOrigin(g_ReplaySystem->m_vecRunData[0].m_vPlayerOrigin);
	SetNextThink(gpGlobals->curtime);

}
void CMomentumReplayGhostEntity::UpdateStep() 
{
    currentStep = g_ReplaySystem->m_vecRunData[step];
    if (mom_replay_reverse.GetBool())
    {
        nextStep = g_ReplaySystem->m_vecRunData[--step];
    }
    else if (step < g_ReplaySystem->m_vecRunData.Size())
    {
        nextStep = g_ReplaySystem->m_vecRunData[++step];
    }
}
void CMomentumReplayGhostEntity::Think(void)
{
	BaseClass::Think();
    if (step >= 0)
    {
        if (step+1 < g_ReplaySystem->m_vecRunData.Size())
        {
            UpdateStep();
            mom_replay_firstperson.GetBool() ? HandleGhostFirstPerson() : HandleGhost();
        } 
        else if (step+1 == g_ReplaySystem->m_vecRunData.Size() && mom_replay_loop.GetBool())
        {
            step = 0; //reset us to the start
        }
        else
        {
            EndRun();
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

    SetNextThink(gpGlobals->curtime + gpGlobals->interval_per_tick);
}
//-----------------------------------------------------------------------------
// Purpose: called by the think function, moves and handles the ghost if we're spectating it
//-----------------------------------------------------------------------------

void CMomentumReplayGhostEntity::HandleGhostFirstPerson()
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer)
    {
        //pPlayer->IsWatchingReplay() = true;
        if (!pPlayer->IsObserver())
        {
            pPlayer->SetObserverTarget(this);
            pPlayer->StartObserverMode(OBS_MODE_IN_EYE);
        }

        if (pPlayer->GetObserverMode() != (OBS_MODE_IN_EYE | OBS_MODE_CHASE)) {
            //we don't want to allow any other obs modes, only IN EYE and CHASE
            pPlayer->ForceObserverMode(OBS_MODE_IN_EYE);
        }
        pPlayer->SetViewOffset(VEC_VIEW);
        SetAbsOrigin(currentStep.m_vPlayerOrigin);

        if (pPlayer->GetObserverMode() == OBS_MODE_IN_EYE) {
            SetAbsAngles(currentStep.m_qEyeAngles);
            // don't render the model when we're in first person mode
            SetRenderMode(kRenderNone);
            AddEffects(EF_NOSHADOW);
        }
        else {
            SetAbsAngles(QAngle(currentStep.m_qEyeAngles.x / 10, //we divide x angle (pitch) by 10 so the ghost doesn't look really stupid
                currentStep.m_qEyeAngles.y, currentStep.m_qEyeAngles.z));
            //remove the nodraw effects
            SetRenderMode(kRenderTransColor);
            RemoveEffects(EF_NOSHADOW);
        }

        //interpolate vel from difference in origin
        float distX = fabs(currentStep.m_vPlayerOrigin.x - nextStep.m_vPlayerOrigin.x);
        float distY = fabs(currentStep.m_vPlayerOrigin.y - nextStep.m_vPlayerOrigin.y);
        float distZ = fabs(currentStep.m_vPlayerOrigin.z - nextStep.m_vPlayerOrigin.z);
        Vector interpolatedVel = Vector(distX, distY, distZ) / gpGlobals->interval_per_tick;
        SetAbsVelocity(interpolatedVel);
        m_nReplayButtons = currentStep.m_nPlayerButtons; //networked var that allows the replay to control keypress display on the client

        if (g_Timer->IsRunning())
            UpdateStats(interpolatedVel);

        if (currentStep.m_nPlayerButtons & IN_DUCK)
        {
            //MOM_TODO: make this smoother. possibly inherit from NPC classes/CBaseCombatCharacter
            pPlayer->SetViewOffset(VEC_DUCK_VIEW);
        }
    }
}

void CMomentumReplayGhostEntity::HandleGhost() 
{
    SetAbsOrigin(currentStep.m_vPlayerOrigin);
    SetAbsAngles(QAngle(currentStep.m_qEyeAngles.x / 10, //we divide x angle (pitch) by 10 so the ghost doesn't look really stupid
        currentStep.m_qEyeAngles.y, currentStep.m_qEyeAngles.z));
    //remove the nodraw effects
    SetRenderMode(kRenderTransColor);
    RemoveEffects(EF_NOSHADOW);

    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer && pPlayer->IsObserver()) //bring the player out of obs mode if theyre currently observing
    {
        //pPlayer->m_bIsWatchingReplay = false;
        pPlayer->StopObserverMode();
        pPlayer->ForceRespawn();
    }
}

void CMomentumReplayGhostEntity::UpdateStats(Vector ghostVel)
{
    // --- STRAFE SYNC ---
    //calculate strafe sync based on replay ghost's movement, in order to update the player's HUD

    float SyncVelocity = ghostVel.Length2DSqr(); //we always want HVEL for checking velocity sync
    if (GetGroundEntity() == NULL)
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
        m_RunData.m_flStrafeSync = (float(m_nPerfectSyncTicks) / float(m_nStrafeTicks)) * 100.0f; // ticks strafing perfectly / ticks strafing
        m_RunData.m_flStrafeSync2 = (float(m_nAccelTicks) / float(m_nStrafeTicks)) * 100.0f; // ticks gaining speed / ticks strafing
    }

    // --- JUMP AND STRAFE COUNTER ---
    //MOM_TODO: GetGroundEntity is never not null (the replay ghost never "jumps")
    //if (GetGroundEntity() != NULL && currentStep.m_nPlayerButtons & IN_JUMP)
    //    pPlayer->m_PlayerRunStats.m_iStageJumps[0]++;

    if ((currentStep.m_nPlayerButtons & IN_MOVELEFT && !(m_nOldReplayButtons & IN_MOVELEFT)) 
        || (currentStep.m_nPlayerButtons & IN_MOVERIGHT && !(m_nOldReplayButtons & IN_MOVERIGHT)) )
        m_iTotalStrafes++;

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
    if (mom_UTIL->GetColorFromHex(args.ArgS())) {
        m_newGhostColor = *mom_UTIL->GetColorFromHex(args.ArgS());
    }
}
void CMomentumReplayGhostEntity::EndRun()
{
	SetNextThink(-1);
	Remove();
    m_bIsActive = false;
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());

    if (pPlayer && pPlayer->IsObserver())
    {
        pPlayer->StopObserverMode();
        pPlayer->ForceRespawn();
        pPlayer->SetMoveType(MOVETYPE_WALK);
        //pPlayer->m_bIsWatchingReplay = false;
    }
}