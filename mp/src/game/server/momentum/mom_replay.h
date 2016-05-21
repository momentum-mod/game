#ifndef MOM_REPLAY_H
#define MOM_REPLAY_H

#include "cbase.h"
#include "filesystem.h"
#include "utlbuffer.h"

#include "replayformat.h"
#include "mom_player_shared.h"
#include "mom_replay_entity.h"

#define RECORDING_PATH "recordings"
#define END_RECORDING_PAUSE 1.0

class CMomentumReplaySystem : CAutoGameSystemPerFrame
{
  public:
    CMomentumReplaySystem(const char *pName)
        : CAutoGameSystemPerFrame(pName), m_bIsWatchingReplay(false), m_bIsRecording(false), m_bShouldStopRec(false),
          m_nCurrentTick(0), m_fRecEndTime(0), m_player(nullptr), m_fhFileHandle(nullptr), m_buf()
    {
    }

    // inherited member from CAutoGameSystemPerFrame
    void FrameUpdatePostEntityThink() override
    {
        if (m_bIsRecording)
        {
            UpdateRecordingParams(&m_buf);
        }
    }

    void LevelShutdownPostEntity() override
    {
        //Stop a recording if there is one while the level shuts down
        if (m_bIsRecording)
            StopRecording(nullptr, true, 0.0f);
    }

    void BeginRecording(CBasePlayer *pPlayer);
    void StopRecording(CBasePlayer *pPlayer, bool throwaway, bool delay);
    void WriteRecordingToFile(CUtlBuffer *buf);
    replay_header_t CreateHeader();

    replay_frame_t *ReadSingleFrame(FileHandle_t file, const char *filename);
    replay_header_t *ReadHeader(FileHandle_t file, const char *filename);

    void StartReplay(bool firstperson = false);
    void EndReplay();
    bool LoadRun(const char *fileName);
    CUtlVector<replay_frame_t> m_vecRunData;

    //MOM_TODO: Handle the pPlayer pointer passed here or get rid of it
    bool IsRecording(CBasePlayer *pPlayer) const
    { return m_bIsRecording; }

    replay_header_t m_loadedHeader;
    bool m_bIsWatchingReplay;
    void DispatchTimerStateMessage(CBasePlayer *pPlayer, bool started);

  private:
    void UpdateRecordingParams(CUtlBuffer *); // called every game frame after entities think and update

    bool m_bIsRecording;
    bool m_bShouldStopRec;
    int m_nCurrentTick;
    float m_fRecEndTime;

    CMomentumPlayer *m_player;
    CMomentumReplayGhostEntity *m_CurrentReplayGhost;

    replay_frame_t m_currentFrame;
    replay_header_t m_replayHeader;

    FileHandle_t m_fhFileHandle;
    CUtlBuffer m_buf;
};

extern CMomentumReplaySystem *g_ReplaySystem;

#endif // MOM_REPLAY_H