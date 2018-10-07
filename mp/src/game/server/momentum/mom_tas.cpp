#include "cbase.h"
#include "mom_tas.h"
#include "mom_player_shared.h"

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

int CTASRecording::NumberOfFrames() { return m_vecTASData.Size(); }

void CTASRecording::PushHead()
{
    if (m_pPlayer->m_SrvData.m_RunData.m_iStatusOfTAS != TAS_RECORDING)
        return;

    m_vecTASData.AddToHead(FrameOfTASData(m_pPlayer->m_SavedUserCmd.viewangles, m_pPlayer->GetAbsOrigin(),
                                          m_pPlayer->GetViewOffset(), m_pPlayer->GetAbsVelocity()));
}

void CTASRecording::Erase(int iFrames)
{
    if (iFrames < 0)
    {
        Assert("Can't remove negatives frames");
        return;
    }

    m_vecTASData.RemoveMultipleFromHead(iFrames);
}

FrameOfTASData *CTASRecording::GetFrame(int iFrame) { return &m_vecTASData[iFrame]; }
