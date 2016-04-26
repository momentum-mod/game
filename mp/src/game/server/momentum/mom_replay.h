#ifndef MOM_REPLAY_H
#define MOM_REPLAY_H
#include "cbase.h"
#include "filesystem.h"
#include "utlbuffer.h"

#include "mom_player_shared.h"
#include "mom_shareddefs.h"
#include "replayformat.h"

#define RECORDING_PATH "recordings"

class CMomentumReplaySystem : CAutoGameSystemPerFrame
{
public:
    CMomentumReplaySystem(const char *pName) : CAutoGameSystemPerFrame(pName) {}
    virtual void FrameUpdatePostEntityThink() //inherited member from CAutoGameSystemPerFrame
    {
        if (m_bIsRecording)
        {
            m_buf = UpdateRecordingParams();
        }
    }
    void BeginRecording(CBasePlayer *pPlayer);
    void StopRecording(CBasePlayer *pPlayer, bool throwaway, float delay = 1.0f);
    bool IsRecording(CBasePlayer *pPlayer) { return m_bIsRecording; }
    void WriteRecordingToFile(CUtlBuffer &buf);
    replay_header_t CreateHeader();
    void WriteRecordingToFile();

    replay_frame_t* ReadSingleFrame(FileHandle_t file, const char* filename);
    replay_header_t* ReadHeader(FileHandle_t file, const char* filename);

    void StartReplay();
    void EndRun();
    bool LoadRun(const char* fileName);
    CUtlVector<replay_frame_t> m_vecRunData;


private:
    CUtlBuffer *UpdateRecordingParams(); //called every game frame after entities think and update

    bool m_bIsRecording;
    int m_nCurrentTick;

    CMomentumPlayer *m_player;

    replay_frame_t m_currentFrame;
    replay_header_t m_replayHeader;

    FileHandle_t m_fhFileHandle;
    CUtlBuffer *m_buf;
};

extern CMomentumReplaySystem *g_ReplaySystem;

#endif //MOM_REPLAY_H