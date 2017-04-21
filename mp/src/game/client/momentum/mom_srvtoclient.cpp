#include "cbase.h"
#include "interface.h"
#include "c_mom_player.h"
#include "run/run_stats.h"
#include "mom_player_shared.h"

DLL_EXPORT void StdDataToClient(CMomRunStats runStats)
{
    C_MomentumPlayer * pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    if(pPlayer)
    {
        pPlayer->m_RunStats = runStats;
    }
}