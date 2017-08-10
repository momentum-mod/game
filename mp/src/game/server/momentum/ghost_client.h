#pragma once

#include "cbase.h"
#include "mom_player.h"
#include "mom_shareddefs.h"
#include "mom_online_ghost.h"
#include "mom_lobby_system.h"

class CMomentumGhostClient : public CAutoGameSystemPerFrame
{
public:
    CMomentumGhostClient(const char *pName) : CAutoGameSystemPerFrame(pName)
    {
        SetDefLessFunc(m_mapOnlineGhosts);
        m_pInstance = this;
    }

    //bool Init() OVERRIDE; MOM_TODO: Set state variables here?
    void PostInit() OVERRIDE;
    //void LevelInitPreEntity() OVERRIDE;
    void LevelInitPostEntity() OVERRIDE;
    //void LevelShutdownPreEntity() OVERRIDE;
    void LevelShutdownPreEntity() OVERRIDE;
    void FrameUpdatePreEntityThink() OVERRIDE;
    void Shutdown() OVERRIDE;

    void ClearCurrentGhosts(bool);

    static bool CreateNewNetFrame(ghostNetFrame_t &frame);
    static ghostAppearance_t CreateAppearance(CMomentumPlayer* pPlayer) { return pPlayer->m_playerAppearanceProps; }

    static CUtlMap<uint64, CMomentumOnlineGhostEntity*> m_mapOnlineGhosts;

private:
    //static CThreadMutex m_mtxGhostPlayers, m_mtxpPlayer;
    static CMomentumPlayer *m_pPlayer;

    static CMomentumGhostClient *m_pInstance;
};

extern CMomentumGhostClient *g_pMomentumGhostClient;
