#ifndef MOM_REPLAY_H
#define MOM_REPLAY_H

#include "cbase.h"
#include "filesystem.h"
#include "utlbuffer.h"

#include "mom_replay_data.h"
#include "mom_player_shared.h"
#include "mom_replay_manager.h"

#define RECORDING_PATH "recordings"
#define END_RECORDING_PAUSE 1.0

class CMomentumReplayGhostEntity;

class CMomentumReplaySystem : CAutoGameSystemPerFrame
{
public:
    CMomentumReplaySystem(const char *pName) : 
        CAutoGameSystemPerFrame(pName),
        m_bShouldStopRec(false),
        m_nCurrentTick(0), 
        m_fRecEndTime(0), 
        m_player(nullptr)
    {
        m_pReplayManager = new CMomReplayManager();
    }

    virtual ~CMomentumReplaySystem() override
    {
        delete m_pReplayManager;
    }

public:
    // inherited member from CAutoGameSystemPerFrame
    void FrameUpdatePostEntityThink() override
    {
        if (m_pReplayManager->Recording())
            UpdateRecordingParams();
    }

    void LevelShutdownPostEntity() override
    {
        //Stop a recording if there is one while the level shuts down
        if (m_pReplayManager->Recording())
            StopRecording(nullptr, true, false);
    }

    void BeginRecording(CBasePlayer *pPlayer);
    void StopRecording(CBasePlayer *pPlayer, bool throwaway, bool delay);

    void StartReplay(bool firstperson = false);
    void EndReplay();

    inline CMomReplayManager* GetReplayManager() const { return m_pReplayManager; }

private:
    void UpdateRecordingParams(); // called every game frame after entities think and update
    void SetReplayInfo();
    void SetRunStats();

private:
    bool m_bShouldStopRec;
    int m_nCurrentTick;
    float m_fRecEndTime;

    CMomentumPlayer *m_player;
    CMomentumReplayGhostEntity *m_CurrentReplayGhost;//MOM_TODO: Update this to be a CUtlVector so multiple ghosts can be kept track of

    CMomReplayManager* m_pReplayManager;
};

extern CMomentumReplaySystem *g_ReplaySystem;

#endif // MOM_REPLAY_H