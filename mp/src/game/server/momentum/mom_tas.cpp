#include "cbase.h"
#include "mom_player_shared.h"
#include "mom_timer.h"
// clang-format: off
#include "mom_replay_system.h"
#include "mom_tas.h"
// clang-format: on

CON_COMMAND(mom_tas, "Set timer to tas mode.")
{
    auto pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());

    if (pPlayer)
    {
        pPlayer->m_SrvData.m_RunData.m_iRunFlags |= RUNFLAG_TAS;
        pPlayer->m_TASRecords->m_Status = TAS_STOPPED;
    }
}

CON_COMMAND(mom_tas_switch_mode, "Switch between pause & record in the tas mode.")
{
    auto pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());

    if (pPlayer)
    {
        if (pPlayer->m_SrvData.m_RunData.m_iRunFlags & RUNFLAG_TAS)
        {
            if (pPlayer->m_TASRecords->m_Status == TAS_RECORDING)
                pPlayer->m_TASRecords->m_Status = TAS_PAUSE;
            else if (pPlayer->m_TASRecords->m_Status == TAS_PAUSE)
                pPlayer->m_TASRecords->m_Status = TAS_RECORDING;
            else if (pPlayer->m_TASRecords->m_Status == TAS_STOPPED)
                pPlayer->m_TASRecords->m_Status = TAS_PAUSE;
        }
    }
}

CON_COMMAND(mom_tas_goto, "Go to a specific tick in the tas mode.")
{
    auto pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());

    if (pPlayer->m_SrvData.m_RunData.m_iRunFlags & RUNFLAG_TAS && pPlayer->m_TASRecords->m_Status == TAS_PAUSE)
    {
        if (args.ArgC() > 1)
        {
            int frame = Q_atoi(args[1]);

            if (frame > 0 && frame <= pPlayer->m_TASRecords->m_vecTASData.Size())
            {
                pPlayer->m_TASRecords->m_iChosenFrame = frame;
            }
        }
    }
}

static MAKE_CONVAR(mom_tas_timescale, "1.0", FCVAR_NONE,
                   "The timescale of a tas play. > 1 is faster, < 1 is slower. \n", 0.01f, 10.0f);

static MAKE_CONVAR(mom_tas_selection, "1", FCVAR_NONE, "0: Paused, 1: Forwards, 2: Backwards \n", 0, 2);

FrameOfTASData::FrameOfTASData()
{
    m_angViewAngles.Init();
    m_vecPosition.Init();
    m_vecViewOffset.Init();
    m_vecAbsVelocity.Init();
    m_iButtons = 0;
}

FrameOfTASData::FrameOfTASData(const QAngle &angViewAngles, const Vector &vecPosition, const Vector &vecViewOffset,
                               const Vector &vecAbsVelocity, int &iButtons)
{
    m_angViewAngles = angViewAngles;
    m_vecPosition = vecPosition;
    m_vecViewOffset = vecViewOffset;
    m_vecAbsVelocity = vecAbsVelocity;
    m_iButtons = iButtons;
}

FrameOfTASData::~FrameOfTASData()
{
    m_angViewAngles.Init();
    m_vecPosition.Init();
    m_vecViewOffset.Init();
    m_vecAbsVelocity.Init();
}

CTASRecording::CTASRecording(CMomentumPlayer *pPlayer)
{
    m_vecTASData.Purge();
    m_pPlayer = pPlayer;
}

CTASRecording::~CTASRecording()
{
    m_vecTASData.Purge();
    m_pPlayer = nullptr;
}

void CTASRecording::Reset()
{
    m_vecTASData.Purge();
    m_iChosenFrame = 0;
    m_Status = TAS_STOPPED;
}

int CTASRecording::NumberOfFrames() { return m_vecTASData.Size(); }

void CTASRecording::Think()
{
    if (!(m_pPlayer->m_SrvData.m_RunData.m_iRunFlags & RUNFLAG_TAS))
        return;

    m_flTimeScale = mom_tas_timescale.GetFloat();

    if (m_Status == TAS_PAUSE && m_OldStatus == TAS_RECORDING)
    {
        m_iPauseTickCount = gpGlobals->tickcount - g_pMomentumTimer->GetStartTick();
        m_iPauseTickCount--;
    }

    if (m_Status == TAS_RECORDING && m_OldStatus == TAS_PAUSE)
    {
        auto FramesToRemove = m_vecTASData.Size() - m_iChosenFrame;

        if (FramesToRemove > 0)
        {
            int iFrameFirstPos = gpGlobals->tickcount + FramesToRemove - m_iPauseTickCount;

            if (g_ReplaySystem.m_bRecording)
            {
                g_ReplaySystem.m_pRecordingReplay->RemoveFramesMult(
                    g_ReplaySystem.m_pRecordingReplay->GetFrameCount() - FramesToRemove, FramesToRemove);
            }

            Erase(FramesToRemove);

            int iOldStartTimerTick = g_ReplaySystem.m_iStartTimerTick;

            g_ReplaySystem.m_iStartTimerTick = g_pMomentumTimer->GetStartTick() = iFrameFirstPos;

            g_ReplaySystem.m_iStartRecordingTick += g_ReplaySystem.m_iStartTimerTick - iOldStartTimerTick;

            m_pPlayer->m_SrvData.m_RunData.m_iStartTick = gpGlobals->tickcount + FramesToRemove - m_iPauseTickCount;
        }

        // As think function is not called, we must do update it this way.
        m_pPlayer->StdDataToPlayer(&m_pPlayer->m_SrvData);
    }

    if (m_Status == TAS_RECORDING)
    {
        m_vecTASData.AddToTail(FrameOfTASData(m_pPlayer->m_SavedUserCmd.viewangles, m_pPlayer->GetAbsOrigin(),
                                              m_pPlayer->GetViewOffset(), m_pPlayer->GetAbsVelocity(),
                                              m_pPlayer->m_SavedUserCmd.buttons));
        m_iChosenFrame++;
    }
    else if (m_Status == TAS_PAUSE)
    {
        auto FramesToRemove = m_vecTASData.Size() - m_iChosenFrame;

        g_pMomentumTimer->GetStartTick() = gpGlobals->tickcount + FramesToRemove - m_iPauseTickCount;
        m_pPlayer->m_SrvData.m_RunData.m_iStartTick = gpGlobals->tickcount + FramesToRemove - m_iPauseTickCount;

        // As think function is not called, we must do update it this way.
        // m_pPlayer->StdDataToPlayer(&m_pPlayer->m_SrvData);
    }

    m_OldStatus = m_Status;
}

void CTASRecording::Erase(int iFrames)
{
    if (iFrames <= 0 || iFrames > m_vecTASData.Size())
    {
        Assert("Can't remove negatives frames or more than actual frames recorded.");
        return;
    }

    m_vecTASData.RemoveMultipleFromTail(iFrames);
}

FrameOfTASData *CTASRecording::GetFrame(int iFrame) { return &m_vecTASData[iFrame]; }

FrameOfTASData *CTASRecording::GetCurFrame() { return GetFrame(m_iChosenFrame); }

void CTASRecording::Start()
{
    if (m_pPlayer->m_SrvData.m_RunData.m_iRunFlags & RUNFLAG_TAS)
    {
        m_Status = TAS_RECORDING;
    }
}

void CTASRecording::Stop()
{
    if (m_pPlayer->m_SrvData.m_RunData.m_iRunFlags & RUNFLAG_TAS)
    {
        m_Status = TAS_STOPPED;
    }
}
