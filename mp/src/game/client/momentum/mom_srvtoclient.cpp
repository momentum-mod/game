#include "cbase.h"
#include "interface.h"
#include "c_mom_player.h"
#include "run/run_stats.h"
#include "mom_player_shared.h"

DLL_EXPORT void StdDataToClient(CMomRunStats &runStats)
{
    C_MomentumPlayer * pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    if(pPlayer)
    {
        C_MomentumReplayGhostEntity *pGhost = pPlayer->GetReplayEnt();
        if (pGhost)
        {
            memcpy((void*)pGhost->m_RunStats.m_pData, (void*)runStats.m_pData, sizeof(runStats));
        }
        else
        {
            memcpy((void*)pPlayer->m_RunStats.m_pData, (void*)runStats.m_pData, sizeof(runStats));
        }
    }
}