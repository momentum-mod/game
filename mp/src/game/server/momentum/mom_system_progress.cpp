#include "cbase.h"

#include "mom_system_progress.h"

#include "tier0/memdbgon.h"


CMomentumProgress::CMomentumProgress()
    : CAutoGameSystem("CMomentumProgress"), m_CurrentWorld(0)
{
}

void CMomentumProgress::BeatStage(int stage)
{
    m_progresses[m_CurrentWorld] |= (1 << stage);

    // MOM_TODO: Save the progress here
}

void CMomentumProgress::Reset()
{
    for (int i = 0; i < 7; i++)
        m_progresses[i] = 0;
}

bool CMomentumProgress::IsStageBeat(int stage, int world /*= -1*/)
{
    // Default to current world if out of bounds
    if (world < 0 || world > 6)
        world = m_CurrentWorld;

    return (m_progresses[world] & (1 << stage)) == (1 << stage);
}

void CMomentumProgress::PostInit()
{
    // MOM_TODO: Load the progress here
}

void CMomentumProgress::Shutdown()
{
    // MOM_TODO: Save the progress here
}

void CMomentumProgress::LevelShutdownPostEntity()
{

}

CON_COMMAND(mom_prog_beat_stage, "Tests the Beat Stage code.")
{
    g_pMomentumProgress->BeatStage(Q_atoi(args.Arg(1)));
}

CON_COMMAND(mom_prog_reset, "Resets progress")
{
    g_pMomentumProgress->Reset();
}

CMomentumProgress s_Progress;
CMomentumProgress *g_pMomentumProgress = &s_Progress;