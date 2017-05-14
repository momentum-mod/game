#include "cbase.h"
#include "run/run_stats.h"
#include "mom_player_shared.h"
#include "mom_modulecomms.h"

#ifdef CLIENT_DLL

#include "c_mom_player.h"
#include "c_mom_replay_entity.h"
StdDataBuffer g_MomServerDataBuf;
StdReplayDataBuffer g_MomReplayDataBuf;

DLL_EXPORT void StdDataToPlayer(StdDataFromServer *from)
{
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    if(pPlayer)
    {
        g_MomServerDataBuf._mutex.Lock();
        memcpy(&g_MomServerDataBuf, from, sizeof(StdDataFromServer));
        g_MomServerDataBuf._mutex.Unlock();
    }
}

DLL_EXPORT void StdDataToReplay(StdReplayDataFromServer *from)
{
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    if(pPlayer)
    {
        C_MomentumReplayGhostEntity *pGhost = pPlayer->GetReplayEnt();
        if (pGhost)
        {
            g_MomReplayDataBuf._mutex.Lock();
            memcpy(&g_MomReplayDataBuf, from, sizeof(StdReplayDataFromServer));
            g_MomReplayDataBuf._mutex.Unlock();
        }
    }
}

void FetchStdData(C_MomentumPlayer *pPlayer)
{
    if(pPlayer)
    {
        g_MomServerDataBuf._mutex.Lock();
        memcpy(&pPlayer->m_SrvData, &g_MomServerDataBuf, sizeof(StdDataFromServer));
        g_MomServerDataBuf._mutex.Unlock();
    }
}

void FetchStdReplayData(C_MomentumReplayGhostEntity *pGhost)
{
    if (pGhost)
    {
        g_MomReplayDataBuf._mutex.Lock();
        memcpy(&pGhost->m_SrvData, &g_MomReplayDataBuf, sizeof(StdReplayDataFromServer));
        g_MomReplayDataBuf._mutex.Unlock();
    }
}

#else //Is not CLIENT_DLL

/*StdDataFromServer::StdDataFromServer()
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(GetLocalPlayer());
    pPlayer->m_RunStats.Init();
}*/

#endif //CLIENT_DLL