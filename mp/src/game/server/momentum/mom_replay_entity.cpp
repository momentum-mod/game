#include "cbase.h"
#include "mom_replay_system.h"
#include "mom_replay_entity.h"
#include "mom_shareddefs.h"
#include "util/mom_util.h"
#include "Timer.h"

#include "tier0/memdbgon.h"

MAKE_TOGGLE_CONVAR(mom_replay_reverse, "0", FCVAR_CLIENTCMD_CAN_EXECUTE, "Reverse playback of replay");
static ConVar mom_replay_ghost_bodygroup("mom_replay_ghost_bodygroup", "11",
                                         FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                         "Replay ghost's body group (model)", true, 0, true, 14);
static ConCommand mom_replay_ghost_color("mom_replay_ghost_color", CMomentumReplayGhostEntity::SetGhostColor,
                                         "Set the ghost's color. Accepts HEX color value in format RRGGBB",
                                         FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE);
static ConVar mom_replay_ghost_alpha("mom_replay_ghost_alpha", "75", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                     "Sets the ghost's transparency, integer between 0 and 255,", true, 0, true, 255);

LINK_ENTITY_TO_CLASS(mom_replay_ghost, CMomentumReplayGhostEntity);

IMPLEMENT_SERVERCLASS_ST(CMomentumReplayGhostEntity, DT_MOM_ReplayEnt)
// MOM_TODO: Network other variables that the UI will need to reference
SendPropInt(SENDINFO(m_nReplayButtons)), 
SendPropInt(SENDINFO(m_iTotalStrafes)), 
SendPropInt(SENDINFO(m_iTotalJumps)),
SendPropFloat(SENDINFO(m_flTickRate)),
SendPropString(SENDINFO(m_pszPlayerName)),
SendPropDataTable(SENDINFO_DT(m_RunData), &REFERENCE_SEND_TABLE(DT_MOM_RunEntData)), 
SendPropDataTable(SENDINFO_DT(m_RunStats), &REFERENCE_SEND_TABLE(DT_MOM_RunStats)),
END_SEND_TABLE();

BEGIN_DATADESC(CMomentumReplayGhostEntity)
END_DATADESC()

Color CMomentumReplayGhostEntity::m_NewGhostColor = COLOR_GREEN;

CMomentumReplayGhostEntity::CMomentumReplayGhostEntity() : 
    m_bIsActive(false),
    m_bReplayShouldLoop(false),
    m_bReplayFirstPerson(false), 
    m_pPlaybackReplay(nullptr), 
    m_bHasJumped(false), 
    m_flLastSyncVelocity(0), 
    m_nStrafeTicks(0),
    m_nPerfectSyncTicks(0),
    m_nAccelTicks(0),
    m_nOldReplayButtons(0)
{
    //Set networked vars here
    m_nReplayButtons = 0;
    m_iTotalStrafes = 0;
    m_RunStats.Init();
    ListenForGameEvent("mapfinished_panel_closed");
}

CMomentumReplayGhostEntity::~CMomentumReplayGhostEntity() 
{ 
}

void CMomentumReplayGhostEntity::Precache(void)
{
    BaseClass::Precache();
    PrecacheModel(GHOST_MODEL);
    m_GhostColor = COLOR_GREEN; // default color
}

void CMomentumReplayGhostEntity::FireGameEvent(IGameEvent* pEvent)
{
    if (!Q_strcmp(pEvent->GetName(), "mapfinished_panel_closed"))
    {
        EndRun();
    }
}

//-----------------------------------------------------------------------------
// Purpose: Sets up the entity's initial state
//-----------------------------------------------------------------------------
void CMomentumReplayGhostEntity::Spawn(void)
{
    Precache();
    BaseClass::Spawn();
    RemoveEffects(EF_NODRAW);
    SetRenderMode(kRenderTransColor);
    SetRenderColor(m_GhostColor.r(), m_GhostColor.g(), m_GhostColor.b(), 75);
    //~~~The magic combo~~~ (collides with triggers, not with players)
    ClearSolidFlags();
    SetCollisionGroup(COLLISION_GROUP_DEBRIS_TRIGGER);
    SetMoveType(MOVETYPE_STEP);
    SetSolid(SOLID_BBOX);
    RemoveSolidFlags(FSOLID_NOT_SOLID);

    SetModel(GHOST_MODEL);
    SetBodygroup(1, mom_replay_ghost_bodygroup.GetInt());

	if (m_pPlaybackReplay)
	{
		engine->ClientCommand(UTIL_GetLocalPlayer()->edict(), "replayui");
		Q_strcpy(m_pszPlayerName.GetForModify(), m_pPlaybackReplay->GetPlayerName());
	}
}

void CMomentumReplayGhostEntity::StartRun(bool firstPerson, bool shouldLoop /* = false */)
{
    m_bReplayFirstPerson = firstPerson;
    m_bReplayShouldLoop = shouldLoop;
    
    Spawn();
    m_iTotalStrafes = 0;
    m_RunData.m_bMapFinished = false;
    m_bIsActive = true;
    m_bHasJumped = false;

    if (m_pPlaybackReplay)
    {
        if (m_bReplayFirstPerson)
        {
            CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
            if (pPlayer && pPlayer->GetReplayEnt() != this)
            {
                pPlayer->SetObserverTarget(this);
                pPlayer->StartObserverMode(OBS_MODE_IN_EYE);
            }
        }

        shared->m_iCurrentTick = mom_replay_reverse.GetBool() ? m_pPlaybackReplay->GetFrameCount() - 1 : 0;
        SetAbsOrigin(m_pPlaybackReplay->GetFrame(shared->m_iCurrentTick)->PlayerOrigin());
		shared->m_iTotalTicks = m_pPlaybackReplay->GetFrameCount() - 1;

        SetNextThink(gpGlobals->curtime);
    }
    else
    {
        Warning("CMomentumReplayGhostEntity::StartRun: No playback replay found!\n");
        EndRun();
    }
}

void CMomentumReplayGhostEntity::UpdateStep()
{
    if (!m_pPlaybackReplay)
        return;

	if (!shared->m_bIsPlaying)
		return;

    if (mom_replay_reverse.GetBool())
    {
        --shared->m_iCurrentTick;

        if (m_bReplayShouldLoop && shared->m_iCurrentTick < 0)
        {
            shared->m_iCurrentTick = m_pPlaybackReplay->GetFrameCount() - 1;
        }

        return;
    }
    
    ++shared->m_iCurrentTick;

    if (shared->m_iCurrentTick >= m_pPlaybackReplay->GetFrameCount() && m_bReplayShouldLoop)
        shared->m_iCurrentTick = 0;
}
void CMomentumReplayGhostEntity::Think(void)
{
    if (!m_bIsActive)
        return;

    if (!m_pPlaybackReplay)
    {
        BaseClass::Think();
        return;
    }

    // update color, bodygroup, and other params if they change
    if (mom_replay_ghost_bodygroup.GetInt() != m_iBodyGroup)
    {
        m_iBodyGroup = mom_replay_ghost_bodygroup.GetInt();
        SetBodygroup(1, m_iBodyGroup);
    }
    if (m_GhostColor != m_NewGhostColor)
    {
        m_GhostColor = m_NewGhostColor;
        SetRenderColor(m_GhostColor.r(), m_GhostColor.g(), m_GhostColor.b());
    }
    if (mom_replay_ghost_alpha.GetInt() != m_GhostColor.a())
    {
        m_GhostColor.SetColor(m_GhostColor.r(), m_GhostColor.g(),
            m_GhostColor.b(), // we have to set the previous colors in order to change alpha...
            mom_replay_ghost_alpha.GetInt());
        SetRenderColorA(mom_replay_ghost_alpha.GetInt());
    }

    bool reverse = mom_replay_reverse.GetBool();

    //move the ghost
    if (!m_bReplayShouldLoop &&
        (reverse && (shared->m_iCurrentTick - 1 < 0) ||
        !reverse && (shared->m_iCurrentTick + 1 >= m_pPlaybackReplay->GetFrameCount())))
    {
        // If we're not looping and we've reached the end of the video then stop and wait for the player
        // to make a choice about if it should repeat, or end.
        SetAbsVelocity(vec3_origin);
    }
    else
    {
        // Otherwise proceed to the next step and perform the necessary updates.
        UpdateStep();
        if (m_rgSpectators.IsEmpty())
            HandleGhost();
        else
            HandleGhostFirstPerson();//MOM_TODO: If some players aren't spectating this, they won't have it update...
    }

    BaseClass::Think();
    SetNextThink(gpGlobals->curtime + gpGlobals->interval_per_tick);
}
//-----------------------------------------------------------------------------
// Purpose: called by the think function, moves and handles the ghost if we're spectating it
//-----------------------------------------------------------------------------
void CMomentumReplayGhostEntity::HandleGhostFirstPerson()
{
    FOR_EACH_VEC(m_rgSpectators, i)
    {
        CMomentumPlayer *pPlayer = m_rgSpectators[i];
        if (pPlayer)
        {
            auto currentStep = GetCurrentStep();
            auto nextStep = GetNextStep();

            if (pPlayer->GetObserverMode() != (OBS_MODE_IN_EYE | OBS_MODE_CHASE))
            {
                // we don't want to allow any other obs modes, only IN EYE and CHASE
                pPlayer->ForceObserverMode(OBS_MODE_IN_EYE);
            }

            pPlayer->SetViewOffset(VEC_VIEW);
            Vector origin = currentStep->PlayerOrigin();
            origin.z -= 3.5f;
            SetAbsOrigin(origin);

            if (pPlayer->GetObserverMode() == OBS_MODE_IN_EYE)
            {
                SetAbsAngles(currentStep->EyeAngles());
                // don't render the model when we're in first person mode
                SetRenderMode(kRenderNone);
                AddEffects(EF_NOSHADOW);
            }
            else
            {
                SetAbsAngles(QAngle(currentStep->EyeAngles().x /
                    10, // we divide x angle (pitch) by 10 so the ghost doesn't look really stupid
                    currentStep->EyeAngles().y,
                    currentStep->EyeAngles().z));

                // remove the nodraw effects
                SetRenderMode(kRenderTransColor);
                RemoveEffects(EF_NOSHADOW);
            }

            // interpolate vel from difference in origin
            float distX = fabs(currentStep->PlayerOrigin().x - nextStep->PlayerOrigin().x);
            float distY = fabs(currentStep->PlayerOrigin().y - nextStep->PlayerOrigin().y);
            float distZ = fabs(currentStep->PlayerOrigin().z - nextStep->PlayerOrigin().z);
            Vector interpolatedVel = Vector(distX, distY, distZ) / gpGlobals->interval_per_tick;
            float maxvel = sv_maxvelocity.GetFloat();

            //Fixes an issue with teleporting
            if (interpolatedVel.x <= maxvel && interpolatedVel.y <= maxvel && interpolatedVel.z <= maxvel)
                SetAbsVelocity(interpolatedVel);

            m_nReplayButtons = currentStep->PlayerButtons(); // networked var that allows the replay to control keypress display on the client

            if (m_RunData.m_bTimerRunning)
                UpdateStats(interpolatedVel);

            if (currentStep->PlayerButtons() & IN_DUCK)
            {
                // MOM_TODO: make this smoother. possibly inherit from NPC classes/CBaseCombatCharacter
                pPlayer->SetViewOffset(VEC_DUCK_VIEW);
            }
        }
    }
}

void CMomentumReplayGhostEntity::HandleGhost()
{
    auto currentStep = GetCurrentStep();

    SetAbsOrigin(currentStep->PlayerOrigin());
    SetAbsAngles(QAngle(
        currentStep->EyeAngles().x / 10, // we divide x angle (pitch) by 10 so the ghost doesn't look really stupid
        currentStep->EyeAngles().y, currentStep->EyeAngles().z));

    // remove the nodraw effects
    SetRenderMode(kRenderTransColor);
    RemoveEffects(EF_NOSHADOW);
}

void CMomentumReplayGhostEntity::UpdateStats(Vector ghostVel)
{
    // --- STRAFE SYNC ---
    // calculate strafe sync based on replay ghost's movement, in order to update the player's HUD

    auto currentStep = GetCurrentStep();
    float SyncVelocity = ghostVel.Length2DSqr(); // we always want HVEL for checking velocity sync

    if (GetGroundEntity() == nullptr)            // The ghost is in the air
    {
        m_bHasJumped = false;

        if (EyeAngles().y > m_angLastEyeAngle.y) // player turned left
        {
            m_nStrafeTicks++;
            if ((currentStep->PlayerButtons() & IN_MOVELEFT) && !(currentStep->PlayerButtons() & IN_MOVERIGHT))
                m_nPerfectSyncTicks++;
            if (SyncVelocity > m_flLastSyncVelocity)
                m_nAccelTicks++;
        }
        else if (EyeAngles().y < m_angLastEyeAngle.y) // player turned right
        {
            m_nStrafeTicks++;
            if ((currentStep->PlayerButtons() & IN_MOVERIGHT) && !(currentStep->PlayerButtons() & IN_MOVELEFT))
                m_nPerfectSyncTicks++;
            if (SyncVelocity > m_flLastSyncVelocity)
                m_nAccelTicks++;
        }
    }
    if (m_nStrafeTicks && m_nAccelTicks && m_nPerfectSyncTicks)
    {
        m_RunData.m_flStrafeSync =
            (float(m_nPerfectSyncTicks) / float(m_nStrafeTicks)) * 100.0f; // ticks strafing perfectly / ticks strafing
        m_RunData.m_flStrafeSync2 =
            (float(m_nAccelTicks) / float(m_nStrafeTicks)) * 100.0f; // ticks gaining speed / ticks strafing
    }

    // --- JUMP AND STRAFE COUNTER ---
    // MOM_TODO: This needs to calculate better. It currently counts every other jump, and sometimes spams (player on
    // ground for a while)
    if (!m_bHasJumped && GetGroundEntity() != nullptr && GetFlags() & FL_ONGROUND &&
        currentStep->PlayerButtons() & IN_JUMP)
    {
        m_bHasJumped = true;
        m_RunData.m_flLastJumpVel = GetLocalVelocity().Length2D();
        m_RunData.m_flLastJumpTime = gpGlobals->curtime;
        m_iTotalJumps++;
    }

    if ((currentStep->PlayerButtons() & IN_MOVELEFT && !(m_nOldReplayButtons & IN_MOVELEFT)) ||
        (currentStep->PlayerButtons() & IN_MOVERIGHT && !(m_nOldReplayButtons & IN_MOVERIGHT)))
        m_iTotalStrafes++;

    m_flLastSyncVelocity = SyncVelocity;
    m_angLastEyeAngle = EyeAngles();
    m_nOldReplayButtons = currentStep->PlayerButtons();
}
void CMomentumReplayGhostEntity::SetGhostModel(const char *newmodel)
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
        Warning("CMomentumReplayGhostEntity::SetGhostBodyGroup() Error: Could not set bodygroup!");
    }
    else
    {
        m_iBodyGroup = bodyGroup;
        SetBodygroup(1, bodyGroup);
    }
}
void CMomentumReplayGhostEntity::SetGhostColor(const CCommand &args)
{
    if (mom_UTIL->GetColorFromHex(args.ArgS()))
    {
        m_NewGhostColor = *mom_UTIL->GetColorFromHex(args.ArgS());
    }
}

void CMomentumReplayGhostEntity::StartTimer(int m_iStartTick)
{
    m_RunData.m_iStartTick = m_iStartTick;

    FOR_EACH_VEC(m_rgSpectators, i)
    {
        CMomentumPlayer *pPlayer = m_rgSpectators[i];
        if (pPlayer && pPlayer->GetReplayEnt() == this)
        {
            g_Timer->DispatchTimerStateMessage(pPlayer, true);
        }
    }
}

void CMomentumReplayGhostEntity::StopTimer()
{
    FOR_EACH_VEC(m_rgSpectators, i)
    {
        CMomentumPlayer *pPlayer = m_rgSpectators[i];
        if (pPlayer && pPlayer->GetReplayEnt() == this)
        {
            g_Timer->DispatchTimerStateMessage(pPlayer, false);
        }
    }
}

void CMomentumReplayGhostEntity::EndRun()
{ 
    StopTimer();// Stop the timer for all spectating us
    m_bIsActive = false;

    // Make everybody stop spectating me. Goes backwards since players remove themselves.
    // MOM_TODO: Do we want to allow the players to still spectate other runs that may be going?
    FOR_EACH_VEC_BACK(m_rgSpectators, i)
    {
        CMomentumPlayer *pPlayer = m_rgSpectators[i];
        if (pPlayer && pPlayer->GetReplayEnt() == this)
        {
            pPlayer->StopSpectating();
        }
    }

    //Theoretically, m_rgSpectators should be empty here.
    m_rgSpectators.RemoveAll();

    // Remove me from the game (destructs me and deletes this pointer on the next game frame)
    Remove();
}

CReplayFrame* CMomentumReplayGhostEntity::GetNextStep()
{
    int nextStep = shared->m_iCurrentTick;

    if (mom_replay_reverse.GetBool())
    {
        --nextStep;

        if (nextStep < 0)
            nextStep = m_pPlaybackReplay->GetFrameCount() - 1;
    }
    else
    {
        ++nextStep;

        if (nextStep >= m_pPlaybackReplay->GetFrameCount())
            nextStep = 0;
    }

    return m_pPlaybackReplay->GetFrame(nextStep);
}