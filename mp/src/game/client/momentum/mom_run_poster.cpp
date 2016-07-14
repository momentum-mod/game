#include "cbase.h"

#include "mom_run_poster.h"

CRunPoster::CRunPoster()
{
    // We need to listen for "replay_save" and "run_save" gameevents
    g_MOMEventListener->ListenForGameEvent("replay_save");
    g_MOMEventListener->ListenForGameEvent("run_save");
}

CRunPoster::~CRunPoster()
{
}
