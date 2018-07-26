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

bool CMomentumProgress::ShouldEnableBrush(int stage)
{
    return (m_progresses[m_CurrentWorld] & (1 << stage)) == (1 << stage);
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

CON_COMMAND(beat_stage, "Tests the Beat Stage code.")
{
    g_pMomentumProgress->BeatStage(Q_atoi(args.Arg(1)));
}

CMomentumProgress s_Progress;
CMomentumProgress *g_pMomentumProgress = &s_Progress;