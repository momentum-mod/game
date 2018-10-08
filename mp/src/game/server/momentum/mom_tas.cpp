#include "cbase.h"
#include "mom_player_shared.h"
#include "mom_timer.h"
// clang-format: off
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

FrameOfTASData::FrameOfTASData()
{
    m_angViewAngles.Init();
    m_vecPosition.Init();
    m_vecViewOffset.Init();
    m_vecAbsVelocity.Init();
}

FrameOfTASData::FrameOfTASData(const QAngle &angViewAngles, const Vector &vecPosition, const Vector &vecViewOffset,
                               const Vector &vecAbsVelocity)
{
    m_angViewAngles = angViewAngles;
    m_vecPosition = vecPosition;
    m_vecViewOffset = vecViewOffset;
    m_vecAbsVelocity = vecAbsVelocity;
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

    if (m_Status == TAS_PAUSE && m_OldStatus == TAS_RECORDING)
    {
        m_iPauseTickCount = gpGlobals->tickcount - g_pMomentumTimer->GetStartTick();
        m_iPauseTickCount--;
    }

    if (m_Status == TAS_RECORDING && m_OldStatus == TAS_PAUSE)
    {
        auto FramesToRemove = m_vecTASData.Size() - m_iChosenFrame;

        if (FramesToRemove > 0)
            Erase(FramesToRemove);

        // As think function is not called, we must do update it this way.
        m_pPlayer->StdDataToPlayer(&m_pPlayer->m_SrvData); 
    }

    if (m_Status == TAS_RECORDING)
    {
        m_vecTASData.AddToTail(FrameOfTASData(m_pPlayer->m_SavedUserCmd.viewangles, m_pPlayer->GetAbsOrigin(),
                                              m_pPlayer->GetViewOffset(), m_pPlayer->GetAbsVelocity()));
        m_iChosenFrame++;
    }
    else if (m_Status == TAS_PAUSE)
    {
        auto chosenFrame = GetFrame(m_iChosenFrame-1);
        m_pPlayer->Teleport(&chosenFrame->m_vecPosition, &chosenFrame->m_angViewAngles, &chosenFrame->m_vecAbsVelocity);
        m_pPlayer->SetViewOffset(chosenFrame->m_vecViewOffset);

        auto FramesToRemove = m_vecTASData.Size() - m_iChosenFrame;
   
        g_pMomentumTimer->GetStartTick() = gpGlobals->tickcount + FramesToRemove - m_iPauseTickCount;
        m_pPlayer->m_SrvData.m_RunData.m_iStartTick = gpGlobals->tickcount + FramesToRemove - m_iPauseTickCount;

        // As think function is not called, we must do update it this way.
        m_pPlayer->StdDataToPlayer(&m_pPlayer->m_SrvData); 
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

void CTASRecording::Start()
{
    if (m_pPlayer->m_SrvData.m_RunData.m_iRunFlags == RUNFLAG_TAS)
    {
        m_Status = TAS_RECORDING;
    }
}

void CTASRecording::Stop()
{
    if (m_pPlayer->m_SrvData.m_RunData.m_iRunFlags == RUNFLAG_TAS)
    {
        m_Status = TAS_STOPPED;
    }
}
