#include "cbase.h"
#include "filesystem.h"
#include "mom_system_progress.h"
#include "mom_player.h"
#include "mom_triggers.h"
#include "fmtstr.h"

#include "tier0/memdbgon.h"

#define PROGRESS_FILENAME "progress.dat"

CMomentumProgress::CMomentumProgress() : CAutoGameSystem("CMomentumProgress"), m_CurrentWorld(0)
{
}

void CMomentumProgress::BeatStage(int stage, int world /*= -1*/)
{
    // Default to current world if out of bounds
    if (world < 0 || world > 6)
        world = m_CurrentWorld;

    m_progresses[world] |= (1 << stage);

    Save();
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

void CMomentumProgress::Save()
{
    KeyValues *pKv = new KeyValues(PROGRESS_FILENAME);
    pKv->SetInt("CurrentWorld", m_CurrentWorld);
    for (int i = 0; i < 7; i++)
    {
        pKv->SetInt(CFmtStr("World%i", i).Get(), m_progresses[i]);
    }
    pKv->SaveToFile(filesystem, PROGRESS_FILENAME, "MOD");
    pKv->deleteThis();
}

void CMomentumProgress::Load()
{
    KeyValues *pKv = new KeyValues(PROGRESS_FILENAME);
    if (pKv->LoadFromFile(filesystem, PROGRESS_FILENAME, "MOD"))
    {
        m_CurrentWorld = pKv->GetInt("CurrentWorld");
        for (int i = 0; i < 7; i++)
            m_progresses[i] = pKv->GetInt(CFmtStr("World%i", i).Get());
    }
    pKv->deleteThis();
}

void CMomentumProgress::PostInit()
{
    Load();
}

void CMomentumProgress::Shutdown()
{
    Save();
}

void CMomentumProgress::LevelShutdownPostEntity()
{
    Save();
}

CON_COMMAND(mom_prog_beat_stage, "Tests the Beat Stage code.")
{
    g_pMomentumProgress->BeatStage(Q_atoi(args.Arg(1)));
}

CON_COMMAND(mom_prog_reset, "Resets progress")
{
    g_pMomentumProgress->Reset();
}

CON_COMMAND(mom_prog_tele, "Teleports player to the last progress trigger\n")
{
    CMomentumPlayer *pPlayer = CMomentumPlayer::GetLocalPlayer();

    if (!pPlayer)
        return;

    CBaseMomentumTrigger *pTrigger = pPlayer->GetCurrentProgressTrigger();

    if (pTrigger)
    {
        pPlayer->Teleport(&pTrigger->GetAbsOrigin(), nullptr, &vec3_origin);
    }
    else
    {
        DevWarning("mom_prog_tele cannot teleport, CurrentProgressTrigger is null!\n");
    }
}


CMomentumProgress s_Progress;
CMomentumProgress *g_pMomentumProgress = &s_Progress;