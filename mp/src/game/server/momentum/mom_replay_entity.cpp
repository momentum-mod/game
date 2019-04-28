#include "cbase.h"

#include "mom_replay_entity.h"
#include "movevars_shared.h"
#include "mom_timer.h"
#include "mom_triggers.h"
#include "util/mom_util.h"
#include "mom_player_shared.h"
#include "run/mom_replay_base.h"
#include "in_buttons.h"

#include "tier0/memdbgon.h"

static ConVar mom_replay_trail_enable("mom_replay_trail_enable", "0", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                      "Paint a faint beam trail on the replay. 0 = OFF, 1 = ON\n", true, 0, true, 1);

LINK_ENTITY_TO_CLASS(mom_replay_ghost, CMomentumReplayGhostEntity);

IMPLEMENT_SERVERCLASS_ST(CMomentumReplayGhostEntity, DT_MOM_ReplayEnt)
SendPropBool(SENDINFO(m_bIsPaused)),
SendPropInt(SENDINFO(m_iCurrentTick)),
SendPropInt(SENDINFO(m_iTotalTicks), -1, SPROP_UNSIGNED),
END_SEND_TABLE();

BEGIN_DATADESC(CMomentumReplayGhostEntity)
END_DATADESC();

CMomentumReplayGhostEntity::CMomentumReplayGhostEntity()
    : m_bIsActive(false), m_bReplayFirstPerson(false), m_pPlaybackReplay(nullptr), m_bHasJumped(false),
      m_flLastSyncVelocity(0), m_nStrafeTicks(0), m_nPerfectSyncTicks(0), m_nAccelTicks(0), m_nOldReplayButtons(0),
      m_vecLastVel(vec3_origin), m_cvarReplaySelection("mom_replay_selection")
{
    m_RunStats.Init();
    m_bIsPaused = false;
    m_iCurrentTick = 0;
    m_iTotalTicks = 0;
    m_pCurrentSpecPlayer = nullptr;
    ListenForGameEvent("mapfinished_panel_closed");
}

CMomentumReplayGhostEntity::~CMomentumReplayGhostEntity() {}

void CMomentumReplayGhostEntity::Precache(void) { BaseClass::Precache(); }

void CMomentumReplayGhostEntity::FireGameEvent(IGameEvent *pEvent)
{
    if (!Q_strcmp(pEvent->GetName(), "mapfinished_panel_closed"))
    {
        if (pEvent->GetBool("restart"))
        {
            m_Data.m_bMapFinished = false;
        }
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

    // MOM_TODO: Read the appearance data from the header of the replay and set it here?
    // Set the appearance to the player's appearance, pending update after spawned in
    CMomentumPlayer *pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (pPlayer)
    {
        SetGhostAppearance(pPlayer->m_playerAppearanceProps);
        // now that we've set our appearance, the ghost should be visible again.
        SetRenderMode(kRenderTransColor);
        if (m_ghostAppearance.m_bGhostTrailEnable)
        {
            CreateTrail();
        }
    }

    if (m_pPlaybackReplay)
        Q_strncpy(m_szGhostName.GetForModify(), m_pPlaybackReplay->GetPlayerName(), MAX_PLAYER_NAME_LENGTH);
}

void CMomentumReplayGhostEntity::StartRun(bool firstPerson)
{
    m_bReplayFirstPerson = firstPerson;

    Spawn();
    m_Data.m_bMapFinished = false;
    m_bIsActive = true;
    m_bHasJumped = false;
    m_bIsPaused = false;

    if (m_pPlaybackReplay)
    {
        if (m_bReplayFirstPerson)
        {
            if (!m_pCurrentSpecPlayer)
                m_pCurrentSpecPlayer = CMomentumPlayer::GetLocalPlayer();

            if (m_pCurrentSpecPlayer && m_pCurrentSpecPlayer->GetGhostEnt() != this)
            {
                m_pCurrentSpecPlayer->SetObserverTarget(this);
                m_pCurrentSpecPlayer->StartObserverMode(OBS_MODE_IN_EYE);
            }
        }

        if (!CloseEnough(m_Data.m_flTickRate, gpGlobals->interval_per_tick, FLT_EPSILON))
        {
            Warning("The tickrate is not equal (%f -> %f)! Stopping replay.\n", m_Data.m_flTickRate,
                    gpGlobals->interval_per_tick);
            EndRun();
            return;
        }

        m_iCurrentTick = 0;
        SetAbsOrigin(m_pPlaybackReplay->GetFrame(m_iCurrentTick)->PlayerOrigin());

        m_iTotalTicks = m_pPlaybackReplay->GetFrameCount() - 1;

        m_Data.m_iCurrentTrack = m_pPlaybackReplay->GetTrackNumber();

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

    m_iCurrentTick = clamp<int>(m_iCurrentTick, 0, m_iTotalTicks);
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

    if (m_iCurrentTick == m_Data.m_iStartTick)
    {
        m_Data.m_bIsInZone = false;
        m_Data.m_bMapFinished = false;
        m_Data.m_bTimerRunning = true;
        StartTimer(gpGlobals->tickcount);

        // Needed for hud_comparisons
        IGameEvent *pEvent = gameeventmanager->CreateEvent("timer_event");
        if (pEvent)
        {
            pEvent->SetInt("ent", entindex());
            pEvent->SetInt("type", TIMER_EVENT_STARTED);

            gameeventmanager->FireEvent(pEvent);
        }
    }

    float m_flTimeScale = ConVarRef("mom_replay_timescale").GetFloat();

    // move the ghost
    if (m_iCurrentTick < 0 || m_iCurrentTick + 1 >= m_iTotalTicks)
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
                    // BLOCK1

                    // If the average of next steps are higher than current steps, the current step must be called here.
                    // Otherwhise the next step must be called.

                    UpdateStep(bShouldNextStepInstead ? (iNextStep - 1) : iNextStep);

                    // Reset our elapsed ticks, to know when we will perform a new current step or a new next step.
                    // At tick 1, because we're increasing only elapsed ticks after the condition of 1) and not before.
                    // If we don't do this, we will be in late of 1 tick.

                    /* --------------------------------------------------------------------------------------------------------------------------
                    For example if m_flTimeScale = 3,5 -> then iInvTicksAverage is equal to 2 (1/0.5), and that we're
                    resetting iTickElapsed on 0, it means that we will wait 2 ticks before being on that BLOCK1. And we
                    dont want that because, we want the 1/2 of time the code running on both blocks and not 1/3 on
                    BLOCK1 then 2/3 on BLOCK2, when timescale is 3,5. If we wait 2 ticks on BLOCK2 and only 1 on BLOCK1,
                    logically, it won't correspond to 3,5 of m_flTimeScale. So we're doing like this way: iTickElapsed =
                    1, or iInvTicksAverage = iInvTicksAverage - 1, to make it correspond perfectly to timescale. I hope
                    you understood what I've meant. If not then contact that XutaxKamay ***** and tell him to fix his
                    comments.
                    ------------------------------------------------------------------------------------------------------------------------------
                    */

                    m_iTickElapsed = 1;
                }
                else
                {

                    // BLOCK2

                    // If the average of next steps are higher than current steps, the next step must be called here.
                    // Otherwhise the current step must be called.

                    UpdateStep(bShouldNextStepInstead ? (iNextStep) : (iNextStep - 1));

                    // Wait for the ticks elapsing before we change to our current step or our next step.
                    m_iTickElapsed++;
                }
            }
        }

        if (m_pCurrentSpecPlayer)
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
}

//-----------------------------------------------------------------------------
// Purpose: called by the think function, moves and handles the ghost if we're spectating it
//-----------------------------------------------------------------------------
void CMomentumReplayGhostEntity::HandleGhostFirstPerson()
{
    if (m_pCurrentSpecPlayer)
    {
        CReplayFrame *currentStep = nullptr;
        CReplayFrame *nextStep = nullptr;
        CReplayFrame *prevStep = nullptr;

        // MOM_TODO
        // If the player is in practice, let's stuck the player, if the current tick is between the start and end of
        // timestamps.
        /*if (m_SrvData.m_iCurrentTick >= m_iPracticeTimeStampStart &&
            m_SrvData.m_iCurrentTick <= m_iPracticeTimeStampEnd)
        {
            m_SrvData.m_bHasPracticeMode = true;
            // To get the real tick of where the practice mode have been enabled, we need to add the tick remainder,
            // but since we want to be stuck at the start of the timestamp and not in the end of our timestamp, we need
            // to get how many ticks have elapsed between those two, and substract it. Usually this calculation is
            // useless if m_iTickRemainder had only increment once in the loop for getting the timestamps.
            nextStep = currentStep = m_pPlaybackReplay->GetFrame(
                m_iPracticeTimeStampStart - (m_iTickRemainder - (m_iPracticeTimeStampEnd - m_iPracticeTimeStampStart)));
        }
        else*/
        {
            // Otherwhise process normally.
            nextStep = GetNextStep();
            currentStep = GetCurrentStep();
            prevStep = GetPreviousStep();

        }

        SetAbsOrigin(currentStep->PlayerOrigin());

        QAngle angles = currentStep->EyeAngles();

        if (m_pCurrentSpecPlayer->GetObserverMode() == OBS_MODE_IN_EYE)
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


        bool bTeleportedThisFrame = (m_cvarReplaySelection.GetInt() == 1) // Going backwards?
            ? prevStep->Teleported() : currentStep->Teleported();

        bool bTeleportedNextFrame = nextStep->Teleported();

        Vector interpolatedVel;
        const float maxvel = sv_maxvelocity.GetFloat();

        if (!bTeleportedNextFrame)
        {
            // interpolate vel from difference in origin
            const Vector &pPlayerCurrentOrigin = currentStep->PlayerOrigin();
            const Vector &pPlayerNextOrigin = nextStep->PlayerOrigin();
            const float distX = fabs(pPlayerCurrentOrigin.x - pPlayerNextOrigin.x);
            const float distY = fabs(pPlayerCurrentOrigin.y - pPlayerNextOrigin.y);
            const float distZ = fabs(pPlayerCurrentOrigin.z - pPlayerNextOrigin.z);
            interpolatedVel = Vector(distX, distY, distZ) / gpGlobals->interval_per_tick;

            m_vecLastVel = interpolatedVel;
        }
        else
        {
            // HACK: Can't interpolate so just assume last one.
            interpolatedVel = m_vecLastVel;
        }

        if (bTeleportedThisFrame)
        {
            // Fix teleporting being interpolated.
            IncrementInterpolationFrame();
        }


        // Fixes an issue with teleporting
        if (interpolatedVel.x <= maxvel && interpolatedVel.y <= maxvel && interpolatedVel.z <= maxvel)
            SetAbsVelocity(interpolatedVel);

        // networked var that allows the replay to control keypress display on the client
        m_nGhostButtons = currentStep->PlayerButtons();

        if (m_Data.m_bTimerRunning)
            UpdateStats(interpolatedVel);

        SetViewOffset(Vector(0, 0, currentStep->PlayerViewOffset()));

        bool isDucking = (GetFlags() & FL_DUCKING) != 0;
        if (m_nGhostButtons & IN_DUCK)
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
    CReplayFrame *currentStep = nullptr;

    // If the player is in practice, let's stuck the player, if the current tick is between the start and end of
    // timestamps.
    /*if (m_SrvData.m_iCurrentTick >= m_iPracticeTimeStampStart && m_SrvData.m_iCurrentTick <= m_iPracticeTimeStampEnd)
    {
        m_SrvData.m_bHasPracticeMode = true;
        // To get the real tick of where the practice mode have been enabled, we need to add the tick remainder,
        // but since we want to be stuck at the start of the timestamp and not in the end of our timestamp, we need to
        // get how many ticks have elapsed between those two, and substract it. Usually this calculation is useless if
        // m_iTickRemainder had only increment once in the loop for getting the timestamps.
        currentStep = m_pPlaybackReplay->GetFrame(
            m_iPracticeTimeStampStart - (m_iTickRemainder - (m_iPracticeTimeStampEnd - m_iPracticeTimeStampStart)));
    }
    else*/
    {
        // Otherwhise process normally.
        currentStep = GetCurrentStep();

    }

    SetAbsOrigin(currentStep->PlayerOrigin());
    SetAbsAngles(QAngle(currentStep->EyeAngles().x /
                            10, // we divide x angle (pitch) by 10 so the ghost doesn't look really stupid
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
        m_Data.m_flStrafeSync =
            (float(m_nPerfectSyncTicks) / float(m_nStrafeTicks)) * 100.0f; // ticks strafing perfectly / ticks strafing
        m_Data.m_flStrafeSync2 =
            (float(m_nAccelTicks) / float(m_nStrafeTicks)) * 100.0f; // ticks gaining speed / ticks strafing
    }

    // --- JUMP AND STRAFE COUNTER ---
    // MOM_TODO: This needs to hook up to the "player jumped" replay tick event
    /*if (!m_bHasJumped && GetGroundEntity() != nullptr && GetFlags() & FL_ONGROUND &&
        currentStep->PlayerButtons() & IN_JUMP)
    {
        m_bHasJumped = true;
        m_SrvData.m_RunData.m_flLastJumpVel = GetLocalVelocity().Length2D();
        m_SrvData.m_RunData.m_flLastJumpTime = gpGlobals->curtime;
        m_SrvData.m_iTotalJumps++;
    }*/

    /*if ((currentStep->PlayerButtons() & IN_MOVELEFT && !(m_nOldReplayButtons & IN_MOVELEFT)) ||
        (currentStep->PlayerButtons() & IN_MOVERIGHT && !(m_nOldReplayButtons & IN_MOVERIGHT)))
        m_SrvData.m_iTotalStrafes++;*/

    m_flLastSyncVelocity = SyncVelocity;
    m_angLastEyeAngle = EyeAngles();
    m_nOldReplayButtons = currentStep->PlayerButtons();
}

void CMomentumReplayGhostEntity::EndRun()
{
    m_bIsActive = false;

    // Make everybody stop spectating me. Goes backwards since players remove themselves.
    if (m_pCurrentSpecPlayer && m_pCurrentSpecPlayer->GetGhostEnt() == this)
    {
        m_pCurrentSpecPlayer->StopSpectating();
    }

    m_pCurrentSpecPlayer = nullptr;

    // Remove me from the game (destructs me and deletes this pointer on the next game frame)
    Remove();
}

CReplayFrame* CMomentumReplayGhostEntity::GetCurrentStep()
{
    return m_pPlaybackReplay->GetFrame(max(min(m_iCurrentTick, m_pPlaybackReplay->GetFrameCount() - 1), 0));
}

CReplayFrame *CMomentumReplayGhostEntity::GetNextStep()
{
    int nextStep = m_iCurrentTick;

    if ((m_cvarReplaySelection.GetInt() == 1) && m_bIsPaused)
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

CReplayFrame *CMomentumReplayGhostEntity::GetPreviousStep()
{
    int nextStep = m_iCurrentTick;

    if ((m_cvarReplaySelection.GetInt() == 1) && m_bIsPaused)
    {
        ++nextStep;

        nextStep = min(nextStep, m_pPlaybackReplay->GetFrameCount() - 1);
    }
    else
    {
        --nextStep;

        nextStep = max(nextStep, 0);
    }

    return m_pPlaybackReplay->GetFrame(nextStep);
}

void CMomentumReplayGhostEntity::OnZoneEnter(CTriggerZone *pTrigger)
{
     // Zone-specific things first
    switch (pTrigger->GetZoneType())
    {
    case ZONE_TYPE_START:
        m_Data.m_iCurrentTrack = pTrigger->GetTrackNumber();
        m_Data.m_bMapFinished = false;
        m_Data.m_bTimerRunning = false;
        break;
    case ZONE_TYPE_STOP:
        {
            m_Data.m_bMapFinished = true;
            m_Data.m_bTimerRunning = false;

            FinishTimer();
            // MOM_TODO: Maybe play effects if the player is racing against us and lost?
        }
        break;
    case ZONE_TYPE_CHECKPOINT:
        break;
    case ZONE_TYPE_STAGE:

    default:
        break;
    }

    CMomRunEntity::OnZoneEnter(pTrigger);
}

void CMomentumReplayGhostEntity::OnZoneExit(CTriggerZone *pTrigger)
{
    // Zone-specific things first
    switch (pTrigger->GetZoneType())
    {
    case ZONE_TYPE_START:
        break;
    case ZONE_TYPE_STOP:

        break;
    case ZONE_TYPE_CHECKPOINT:
        break;
    case ZONE_TYPE_STAGE:

    default:
        break;
    }

    CMomRunEntity::OnZoneExit(pTrigger);
}


void CMomentumReplayGhostEntity::CreateTrail()
{
    if (!mom_replay_trail_enable.GetBool())
        return;
    BaseClass::CreateTrail();
}
void CMomentumReplayGhostEntity::SetGhostColor(const uint32 newHexColor)
{
    m_ghostAppearance.m_iGhostModelRGBAColorAsHex = newHexColor;
    Color newColor;
    if (g_pMomentumUtil->GetColorFromHex(newHexColor, newColor))
        SetRenderColor(newColor.r(), newColor.g(), newColor.b(), 75);
}
