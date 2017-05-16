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
    g_MomServerDataBuf._mutex.Lock();
    g_MomServerDataBuf.m_bWritten = true;
    memcpy(&g_MomServerDataBuf, from, sizeof(StdDataFromServer));
    g_MomServerDataBuf._mutex.Unlock();
}

DLL_EXPORT void StdDataToReplay(StdReplayDataFromServer *from)
{

    g_MomReplayDataBuf._mutex.Lock();
    g_MomReplayDataBuf.m_bWritten = true;
    memcpy(&g_MomReplayDataBuf, from, sizeof(StdReplayDataFromServer));
    g_MomReplayDataBuf._mutex.Unlock();
}

void FetchStdData(C_MomentumPlayer *pPlayer)
{
    if(pPlayer)
    {
        g_MomServerDataBuf._mutex.Lock();
        if (!g_MomServerDataBuf.m_bWritten)
            return;
        
        g_MomServerDataBuf.m_bWritten = false;
        memcpy(&pPlayer->m_SrvData, &g_MomServerDataBuf, sizeof(StdDataFromServer));
        g_MomServerDataBuf._mutex.Unlock();
    }
}

void FetchStdReplayData(C_MomentumReplayGhostEntity *pGhost)
{
    if (pGhost)
    {
        g_MomReplayDataBuf._mutex.Lock();
        if (!g_MomReplayDataBuf.m_bWritten)
            return;
        
        g_MomReplayDataBuf.m_bWritten = false;
        memcpy(&pGhost->m_SrvData, &g_MomReplayDataBuf, sizeof(StdReplayDataFromServer));
        g_MomReplayDataBuf._mutex.Unlock();
    }
}

#else //Is not CLIENT_DLL


#endif //CLIENT_DLL