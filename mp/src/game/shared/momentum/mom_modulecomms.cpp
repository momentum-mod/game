#include "cbase.h"
#include "run/run_stats.h"
#include "mom_player_shared.h"
#include "mom_modulecomms.h"

#ifdef CLIENT_DLL

#include "c_mom_player.h"

DLL_EXPORT void StdDataToPlayer(StdDataFromServer *from)
{
    C_MomentumPlayer * pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    if(pPlayer)
    {

        memcpy(&pPlayer->m_SrvData, from, sizeof(StdDataFromServer));
    }
}

DLL_EXPORT void StdDataToReplay(StdReplayDataFromServer *from)
{
    C_MomentumPlayer * pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    if(pPlayer)
    {
        C_MomentumReplayGhostEntity *pGhost = pPlayer->GetReplayEnt();
        if (pGhost)
        {
            memcpy(&pGhost->m_SrvData, from, sizeof(StdReplayDataFromServer));
        }
    }
}

#else //Is not CLIENT_DLL

/*StdDataFromServer::StdDataFromServer()
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(GetLocalPlayer());
    pPlayer->m_RunStats.Init();
}*/

#endif //CLIENT_DLL