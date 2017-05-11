#include "cbase.h"

#include "mom_replay_entity.h"
#include "mom_replay_system.h"
#include "mom_shareddefs.h"
#include "mom_timer.h"
#include "util/mom_util.h"
#include "util/os_utils.h"
#ifdef _WIN32
#include <windows.h>
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
SendPropInt(SENDINFO(m_nReplayButtons)),
SendPropInt(SENDINFO(m_iTotalStrafes)),
SendPropInt(SENDINFO(m_iTotalJumps)),
SendPropFloat(SENDINFO(m_flTickRate)),
SendPropString(SENDINFO(m_pszPlayerName)),
SendPropInt(SENDINFO(m_iTotalTimeTicks)),
SendPropInt(SENDINFO(m_iCurrentTick)),
SendPropBool(SENDINFO(m_bIsPaused)),
//SendPropDataTable(SENDINFO_DT(m_RunData), &REFERENCE_SEND_TABLE(DT_MOM_RunEntData)),
//SendPropDataTable(SENDINFO_DT(m_RunStats), &REFERENCE_SEND_TABLE(DT_MOM_RunStats)),
END_SEND_TABLE();

BEGIN_DATADESC(CMomentumReplayGhostEntity)
END_DATADESC();

Color CMomentumReplayGhostEntity::m_NewGhostColor = COLOR_GREEN;

CMomentumReplayGhostEntity::CMomentumReplayGhostEntity()
    : m_bIsActive(false), m_bReplayFirstPerson(false), m_pPlaybackReplay(nullptr), m_iBodyGroup(BODY_PROLATE_ELLIPSE),
    m_bHasJumped(false), m_flLastSyncVelocity(0), m_nStrafeTicks(0), m_nPerfectSyncTicks(0), m_nAccelTicks(0),
    m_nOldReplayButtons(0)
{
    StdDataToReplay = (DataToReplayFn)(GetProcAddress( GetModuleHandle(CLIENT_DLL), "StdDataToReplay"));
    
    // Set networked vars here
    m_nReplayButtons = 0;
    m_iTotalStrafes = 0;
    m_RunStats.m_pData = &(m_SrvData.m_RunStatsData);
    m_RunStats.Init(g_pMomentumTimer->GetZoneCount());
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
    m_iTotalStrafes = 0;
    m_SrvData.m_RunData.m_bMapFinished = false;
    m_bIsActive = true;
    m_bHasJumped = false;
    m_bIsPaused = false;

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

        m_iCurrentTick = 0;
        SetAbsOrigin(m_pPlaybackReplay->GetFrame(m_iCurrentTick)->PlayerOrigin());
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

    if (m_bIsPaused)
    {
        if (ConVarRef("mom_replay_selection").GetInt() == 1)
            m_iCurrentTick -= Skip;
        else if (ConVarRef("mom_replay_selection").GetInt() == 2)
            m_iCurrentTick += Skip;
    }
    else
    {
        m_iCurrentTick += Skip;
    }

    m_iCurrentTick = clamp<int>(m_iCurrentTick, 0, m_iTotalTimeTicks);
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
    if (m_iCurrentTick < 0 || m_iCurrentTick + 1 >= m_pPlaybackReplay->GetFrameCount())
    {
        // If we're not looping and we've reached the end of the video then stop and wait for the player
        // to make a choice about if it should repeat, or end.
        SetAbsVelocity(vec3_origin);
    }
    else
    {
        // MOM_TODO: Find a better solution for timescaling when it's > 1.0
        // (commented old, it could be useful, we must find a better solution for this... host_timescale would work, but
        // only with sv_cheats enabled, wich we would to enable maybe?)
        // It's kinda hard without modifying the engine dll

        // Otherwise proceed to the next step and perform the necessary updates.
        if (m_flTimeScale <= 1.0f)
            UpdateStep(1);
        else
        {
            int NextStep = static_cast<int>(m_flTimeScale)+1;
            UpdateStep(NextStep);
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
        int NextStep = static_cast<int>(m_flTimeScale)+1;

        float CalculateSlowMotion = gpGlobals->interval_per_tick * (NextStep - m_flTimeScale);
        SetNextThink(gpGlobals->curtime + gpGlobals->interval_per_tick + CalculateSlowMotion);
    }
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
        m_nReplayButtons = currentStep->PlayerButtons();

        if (m_SrvData.m_RunData.m_bTimerRunning)
            UpdateStats(interpolatedVel);

        SetViewOffset(currentStep->PlayerViewOffset());

        // kamay: Now timer start and end at the right time
        bool isDucking = (GetFlags() & FL_DUCKING) != 0;
        if (m_nReplayButtons & IN_DUCK)
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
        m_iTotalJumps++;
    }

    if ((currentStep->PlayerButtons() & IN_MOVELEFT && !(m_nOldReplayButtons & IN_MOVELEFT)) ||
        (currentStep->PlayerButtons() & IN_MOVERIGHT && !(m_nOldReplayButtons & IN_MOVERIGHT)))
        m_iTotalStrafes++;

    m_flLastSyncVelocity = SyncVelocity;
    m_angLastEyeAngle = EyeAngles();
    m_nOldReplayButtons = currentStep->PlayerButtons();
    
    //TODO: Can this run only once? Probably. But I can't find a place where that works.
    //This should REALLY be done.
    if (StdDataToReplay) 
        StdDataToReplay(&m_SrvData);
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
    int nextStep = m_iCurrentTick;

    if ((ConVarRef("mom_replay_selection").GetInt() == 1) && m_bIsPaused)
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
