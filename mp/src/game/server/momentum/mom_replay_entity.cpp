#include "cbase.h"

#include "mom_replay_entity.h"
#include "mom_replay_system.h"
#include "mom_shareddefs.h"
#include "mom_timer.h"
#include "util/mom_util.h"
#include "util/os_utils.h"
#ifdef _WIN32
#pragma warning( disable: 4005 )
#include <Windows.h>
#endif
#include "tier0/memdbgon.h"

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
//SendPropInt(SENDINFO(m_nReplayButtons)),
//SendPropInt(SENDINFO(m_iTotalStrafes)),
//SendPropInt(SENDINFO(m_iTotalJumps)),
SendPropFloat(SENDINFO(m_flTickRate)),
SendPropString(SENDINFO(m_pszPlayerName)),
SendPropInt(SENDINFO(m_iTotalTimeTicks)),
//SendPropInt(SENDINFO(m_iCurrentTick)),
//SendPropBool(SENDINFO(m_bIsPaused)),
//SendPropDataTable(SENDINFO_DT(m_RunData), &REFERENCE_SEND_TABLE(DT_MOM_RunEntData)),
//SendPropDataTable(SENDINFO_DT(m_RunStats), &REFERENCE_SEND_TABLE(DT_MOM_RunStats)),
END_SEND_TABLE();

BEGIN_DATADESC(CMomentumReplayGhostEntity)
END_DATADESC();

Color CMomentumReplayGhostEntity::m_NewGhostColor = COLOR_GREEN;

CMomentumReplayGhostEntity::CMomentumReplayGhostEntity()
    : m_bIsActive(false), m_bReplayFirstPerson(false), m_pPlaybackReplay(nullptr), m_iBodyGroup(BODY_PROLATE_ELLIPSE),
    m_bHasJumped(false), m_flLastSyncVelocity(0), m_nStrafeTicks(0), m_nPerfectSyncTicks(0), m_nAccelTicks(0),
    m_nOldReplayButtons(0), m_RunStats(&m_SrvData.m_RunStatsData, g_pMomentumTimer->GetZoneCount())
{
    StdDataToReplay = (DataToReplayFn)(GetProcAddress( GetModuleHandle(CLIENT_DLL), "StdDataToReplay"));
    
    // Set networked vars here
    m_SrvData.m_nReplayButtons = 0;
    m_SrvData.m_iTotalStrafes = 0;
    m_pPlayerSpectator = nullptr;
    ListenForGameEvent("mapfinished_panel_closed");
}

CMomentumReplayGhostEntity::~CMomentumReplayGhostEntity() {}

void CMomentumReplayGhostEntity::Precache(void)
{
    BaseClass::Precache();
    PrecacheModel(GHOST_MODEL);
    m_GhostColor = COLOR_GREEN; // default color
}

void CMomentumReplayGhostEntity::FireGameEvent(IGameEvent *pEvent)
{
    if (!Q_strcmp(pEvent->GetName(), "mapfinished_panel_closed"))
    {
        if (pEvent->GetBool("restart"))
            m_SrvData.m_RunData.m_bMapFinished = false;
        else
            EndRun();
    }
}

//-----------------------------------------------------------------------------
// Purpose: Sets up the entity's initial state
//-----------------------------------------------------------------------------
void CMomentumReplayGhostEntity::Spawn()
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
    // Always call CollisionBounds after you set the model
    SetCollisionBounds(VEC_HULL_MIN, VEC_HULL_MAX);
    SetBodygroup(1, mom_replay_ghost_bodygroup.GetInt());
    UpdateModelScale();
    SetViewOffset(VEC_VIEW_SCALED(this));

    if (m_pPlaybackReplay)
        Q_strcpy(m_pszPlayerName.GetForModify(), m_pPlaybackReplay->GetPlayerName());
}

void CMomentumReplayGhostEntity::StartRun(bool firstPerson)
{
    m_bReplayFirstPerson = firstPerson;

    Spawn();
    m_SrvData.m_iTotalStrafes = 0;
    m_SrvData.m_RunData.m_bMapFinished = false;
    m_bIsActive = true;
    m_bHasJumped = false;
    m_SrvData.m_bIsPaused = false;

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

        if (!g_pMomentumUtil->FloatEquals(m_flTickRate, gpGlobals->interval_per_tick))
        {
            Warning("The tickrate is not equal (%f -> %f)! Stopping replay.\n", m_flTickRate.Get(),
                gpGlobals->interval_per_tick);
            EndRun();
            return;
        }

        m_SrvData.m_iCurrentTick = 0;
        SetAbsOrigin(m_pPlaybackReplay->GetFrame(m_SrvData.m_iCurrentTick)->PlayerOrigin());
        m_iTotalTimeTicks = m_pPlaybackReplay->GetFrameCount() - 1;

        SetNextThink(gpGlobals->curtime + gpGlobals->interval_per_tick);
    }
    else
    {
        Warning("CMomentumReplayGhostEntity::StartRun: No playback replay found!\n");
        EndRun();
    }
}

void CMomentumReplayGhostEntity::UpdateStep(int Skip)
{
    // Managed by replayui now
    if (!m_pPlaybackReplay)
        return;

    if (m_SrvData.m_bIsPaused)
    {
        if (ConVarRef("mom_replay_selection").GetInt() == 1)
            m_SrvData.m_iCurrentTick -= Skip;
        else if (ConVarRef("mom_replay_selection").GetInt() == 2)
            m_SrvData.m_iCurrentTick += Skip;
    }
    else
    {
        m_SrvData.m_iCurrentTick += Skip;
    }

    m_SrvData.m_iCurrentTick = clamp<int>(m_SrvData.m_iCurrentTick, 0, m_iTotalTimeTicks);
}

void CMomentumReplayGhostEntity::Think()
{

    BaseClass::Think();

    if (!m_bIsActive)
        return;

    if (!m_pPlaybackReplay)
    {
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

    float m_flTimeScale = ConVarRef("mom_replay_timescale").GetFloat();

    // move the ghost
    if (m_SrvData.m_iCurrentTick < 0 || m_SrvData.m_iCurrentTick + 1 >= m_pPlaybackReplay->GetFrameCount())
    {
        // If we're not looping and we've reached the end of the video then stop and wait for the player
        // to make a choice about if it should repeat, or end.
        SetAbsVelocity(vec3_origin);
    }
    else
    {
        if (m_flTimeScale <= 1.0f)
            UpdateStep(1);
        else
        {
            // MOM_TODO: IMPORTANT! Remember, this is probably not the proper way of speeding up the replay.
            // Because it skips the steps that normaly the engine would have "compensated".
            // So it can results to unsmooth results, but this is probably the best you can get.
            // Until we can find something else to modify timescale properly.
            // We do it this way, because SetNextThink / engine doesn't allow faster updates at this timescale.

            // Calculate our next step
            int iNextStep = static_cast<int>(m_flTimeScale) + 1;

            // Calculate the average of ticks that will be used for the next step or the current one
            float fTicksAverage = (1.0f - (static_cast<float>(iNextStep) - m_flTimeScale));

            // If it's null, then we just run the current step
            if (fTicksAverage == 0.0f)
            {
                UpdateStep(iNextStep - 1);
            }

            // Otherwhise if it's 1 we must run the next step
            else if (fTicksAverage == 1.0f)
            {
                UpdateStep(iNextStep);
            }

            // Else, we calculate when we should be on the next step or the current one
            else
            {

                // If we should first update on the next step or not
                bool bShouldNextStepInstead = false;

                // If the next step that must be runned is higher than the current steps:
                // We invert roles between current steps and next steps.
                if (fTicksAverage > 0.5f)
                {
                    fTicksAverage = 0.5f - (fTicksAverage - 0.5f);
                    bShouldNextStepInstead = true;
                }

                // Actually we don't need to check for the tickrate, we will let engine compensate it.
                float fInvTicksAverage = 1.0f / fTicksAverage;

                int iInvTicksAverage = static_cast<int>(fInvTicksAverage + 0.5f);

                // 1) If the ticks elapsed is higher or equal to the ticks calculated we must run the next step or the
                // current one depending on the average of current steps and next steps.
                if (m_iTickElapsed >= iInvTicksAverage)
                {
                    //BLOCK1

                    // If the average of next steps are higher than current steps, the current step must be called here.
                    // Otherwhise the next step must be called.

                    UpdateStep(bShouldNextStepInstead ? (iNextStep - 1) : iNextStep);

                    // Reset our elapsed ticks, to know when we will perform a new current step or a new next step.
                    // At tick 1, because we're increasing only elapsed ticks after the condition of 1) and not before.
                    // If we don't do this, we will be in late of 1 tick.

                    /* --------------------------------------------------------------------------------------------------------------------------
                    For example if m_flTimeScale = 3,5 -> then iInvTicksAverage is equal to 2 (1/0.5), and that we're resetting iTickElapsed on 0,
                    it means that we will wait 2 ticks before being on that BLOCK1.
                    And we dont want that because, we want the 1/2 of time the code running on both blocks and not 1/3 on BLOCK1 then 2/3 on BLOCK2,
                    when timescale is 3,5.
                    If we wait 2 ticks on BLOCK2 and only 1 on BLOCK1, logically, it won't correspond to 3,5 of m_flTimeScale.
                    So we're doing like this way: iTickElapsed = 1, or iInvTicksAverage = iInvTicksAverage - 1, 
                    to make it correspond perfectly to timescale.
                    I hope you understood what I've meant. If not then contact that XutaxKamay ***** and tell him to fix his comments.
                    ------------------------------------------------------------------------------------------------------------------------------
                    */

                    m_iTickElapsed = 1;
                }
                else
                {

                    //BLOCK2

                    // If the average of next steps are higher than current steps, the next step must be called here.
                    // Otherwhise the current step must be called.

                    UpdateStep(bShouldNextStepInstead ? (iNextStep) : (iNextStep - 1));

                    // Wait for the ticks elapsing before we change to our current step or our next step.
                    m_iTickElapsed++;
                }
            }
        }

        if (m_pPlayerSpectator)
            HandleGhostFirstPerson();
        else
            HandleGhost();
    }

    if (m_flTimeScale <= 1.0f)
    {
        SetNextThink(gpGlobals->curtime + gpGlobals->interval_per_tick * (1.0f / m_flTimeScale));
    }
    else
    {
        SetNextThink(gpGlobals->curtime + gpGlobals->interval_per_tick);
    }
    
    if (StdDataToReplay)
        StdDataToReplay(&m_SrvData);
}

// Ripped from gamemovement for slightly better collision
inline bool CanUnduck(CMomentumReplayGhostEntity *pGhost)
{
    trace_t trace;
    Vector newOrigin;

    VectorCopy(pGhost->GetAbsOrigin(), newOrigin);

    if (pGhost->GetGroundEntity() != nullptr)
    {
        newOrigin += VEC_DUCK_HULL_MIN - VEC_HULL_MIN;
    }
    else
    {
        // If in air an letting go of croush, make sure we can offset origin to make
        //  up for uncrouching
        Vector hullSizeNormal = VEC_HULL_MAX - VEC_HULL_MIN;
        Vector hullSizeCrouch = VEC_DUCK_HULL_MAX - VEC_DUCK_HULL_MIN;

        newOrigin += -0.5f * (hullSizeNormal - hullSizeCrouch);
    }

    UTIL_TraceHull(pGhost->GetAbsOrigin(), newOrigin, VEC_HULL_MIN, VEC_HULL_MAX, MASK_PLAYERSOLID, pGhost,
        COLLISION_GROUP_PLAYER_MOVEMENT, &trace);

    if (trace.startsolid || (trace.fraction != 1.0f))
        return false;

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: called by the think function, moves and handles the ghost if we're spectating it
//-----------------------------------------------------------------------------
void CMomentumReplayGhostEntity::HandleGhostFirstPerson()
{
    if (m_pPlayerSpectator)
    {
        auto currentStep = GetCurrentStep();
        auto nextStep = GetNextStep();

        if (m_pPlayerSpectator->GetObserverMode() != (OBS_MODE_IN_EYE | OBS_MODE_CHASE))
        {
            // we don't want to allow any other obs modes, only IN EYE and CHASE
            m_pPlayerSpectator->ForceObserverMode(OBS_MODE_IN_EYE);
        }

        SetAbsOrigin(currentStep->PlayerOrigin());

        QAngle angles = currentStep->EyeAngles();

        if (m_pPlayerSpectator->GetObserverMode() == OBS_MODE_IN_EYE)
        {
            SetAbsAngles(angles);
            // don't render the model when we're in first person mode
            if (GetRenderMode() != kRenderNone)
            {
                SetRenderMode(kRenderNone);
                AddEffects(EF_NOSHADOW);
            }
        }
        else
        {
            // we divide x angle (pitch) by 10 so the ghost doesn't look really stupid
            SetAbsAngles(QAngle(angles.x / 10, angles.y, angles.z));

            // remove the nodraw effects
            if (GetRenderMode() != kRenderTransColor)
            {
                SetRenderMode(kRenderTransColor);
                RemoveEffects(EF_NOSHADOW);
            }
        }


        // interpolate vel from difference in origin
        const Vector &pPlayerCurrentOrigin = currentStep->PlayerOrigin();
        const Vector &pPlayerNextOrigin = nextStep->PlayerOrigin();
        const float distX = fabs(pPlayerCurrentOrigin.x - pPlayerNextOrigin.x);
        const float distY = fabs(pPlayerCurrentOrigin.y - pPlayerNextOrigin.y);
        const float distZ = fabs(pPlayerCurrentOrigin.z - pPlayerNextOrigin.z);
        const Vector interpolatedVel = Vector(distX, distY, distZ) / gpGlobals->interval_per_tick;
        const float maxvel = sv_maxvelocity.GetFloat();


        // Fixes an issue with teleporting
        if (interpolatedVel.x <= maxvel && interpolatedVel.y <= maxvel && interpolatedVel.z <= maxvel)
            SetAbsVelocity(interpolatedVel);

        // networked var that allows the replay to control keypress display on the client
        m_SrvData.m_nReplayButtons = currentStep->PlayerButtons();

        if (m_SrvData.m_RunData.m_bTimerRunning)
            UpdateStats(interpolatedVel);

        SetViewOffset(currentStep->PlayerViewOffset());

        // kamay: Now timer start and end at the right time
        bool isDucking = (GetFlags() & FL_DUCKING) != 0;
        if (m_SrvData.m_nReplayButtons & IN_DUCK)
        {
            if (!isDucking)
            {
                SetCollisionBounds(VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
                AddFlag(FL_DUCKING);
            }
        }
        else
        {
            if (CanUnduck(this) && isDucking)
            {
                SetCollisionBounds(VEC_HULL_MIN, VEC_HULL_MAX);
                RemoveFlag(FL_DUCKING);
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

void CMomentumReplayGhostEntity::UpdateStats(const Vector &ghostVel)
{
    // --- STRAFE SYNC ---
    // calculate strafe sync based on replay ghost's movement, in order to update the player's HUD

    auto currentStep = GetCurrentStep();
    float SyncVelocity = ghostVel.Length2DSqr(); // we always want HVEL for checking velocity sync

    if (GetGroundEntity() == nullptr) // The ghost is in the air
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
        m_SrvData.m_RunData.m_flStrafeSync =
            (float(m_nPerfectSyncTicks) / float(m_nStrafeTicks)) * 100.0f; // ticks strafing perfectly / ticks strafing
        m_SrvData.m_RunData.m_flStrafeSync2 =
            (float(m_nAccelTicks) / float(m_nStrafeTicks)) * 100.0f; // ticks gaining speed / ticks strafing
    }

    // --- JUMP AND STRAFE COUNTER ---
    // MOM_TODO: This needs to calculate better. It currently counts every other jump, and sometimes spams (player on
    // ground for a while)
    if (!m_bHasJumped && GetGroundEntity() != nullptr && GetFlags() & FL_ONGROUND &&
        currentStep->PlayerButtons() & IN_JUMP)
    {
        m_bHasJumped = true;
        m_SrvData.m_RunData.m_flLastJumpVel = GetLocalVelocity().Length2D();
        m_SrvData.m_RunData.m_flLastJumpTime = gpGlobals->curtime;
        m_SrvData.m_iTotalJumps++;
    }

    if ((currentStep->PlayerButtons() & IN_MOVELEFT && !(m_nOldReplayButtons & IN_MOVELEFT)) ||
        (currentStep->PlayerButtons() & IN_MOVERIGHT && !(m_nOldReplayButtons & IN_MOVERIGHT)))
        m_SrvData.m_iTotalStrafes++;

    m_flLastSyncVelocity = SyncVelocity;
    m_angLastEyeAngle = EyeAngles();
    m_nOldReplayButtons = currentStep->PlayerButtons();
}
void CMomentumReplayGhostEntity::SetGhostModel(const char *newmodel)
{
    if (newmodel)
    {
        Q_strncpy(m_pszModel, newmodel, sizeof(m_pszModel));
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
    Color *pColor = g_pMomentumUtil->GetColorFromHex(args.ArgS());
    if (pColor)
    {
        m_NewGhostColor = *pColor;
    }
}

void CMomentumReplayGhostEntity::StartTimer(int m_iStartTick)
{
    m_SrvData.m_RunData.m_iStartTick = m_iStartTick;

    if (m_pPlayerSpectator && m_pPlayerSpectator->GetReplayEnt() == this)
    {
        g_pMomentumTimer->DispatchTimerStateMessage(m_pPlayerSpectator, true);
    }
}

void CMomentumReplayGhostEntity::StopTimer()
{
    if (m_pPlayerSpectator && m_pPlayerSpectator->GetReplayEnt() == this)
    {
        g_pMomentumTimer->DispatchTimerStateMessage(m_pPlayerSpectator, false);
    }
}

void CMomentumReplayGhostEntity::EndRun()
{
    StopTimer(); // Stop the timer for all spectating us
    m_bIsActive = false;

    // Make everybody stop spectating me. Goes backwards since players remove themselves.
    if (m_pPlayerSpectator && m_pPlayerSpectator->GetReplayEnt() == this)
    {
        m_pPlayerSpectator->StopSpectating();
    }

    m_pPlayerSpectator = nullptr;

    // Remove me from the game (destructs me and deletes this pointer on the next game frame)
    Remove();
}

CReplayFrame *CMomentumReplayGhostEntity::GetNextStep()
{
    int nextStep = m_SrvData.m_iCurrentTick;

    if ((ConVarRef("mom_replay_selection").GetInt() == 1) && m_SrvData.m_bIsPaused)
    {
        --nextStep;

        nextStep = max(nextStep, 0);
    }
    else
    {
        ++nextStep;

        nextStep = min(nextStep, m_pPlaybackReplay->GetFrameCount() - 1);
    }

    return m_pPlaybackReplay->GetFrame(nextStep);
}
