#ifndef MOM_REPLAY_H
#define MOM_REPLAY_H
#include "cbase.h"
#include "filesystem.h"
#include "utlbuffer.h"

#include "mom_player_shared.h"
#include "mom_shareddefs.h"

class CMomentumReplaySystem : CAutoGameSystemPerFrame
{
public:
    CMomentumReplaySystem(const char *pName) : CAutoGameSystemPerFrame(pName) {}
    virtual void FrameUpdatePostEntityThink()
    {
        if (m_bIsRecording)
        {
            buf = UpdateRecordingParams();
        }
    }
    void BeginRecording(CBasePlayer *pPlayer);
    void StopRecording(CBasePlayer *pPlayer, bool throwaway);
    bool IsRecording(CBasePlayer *pPlayer) { return m_bIsRecording; }
    void WriteRecordingToFile(CUtlBuffer &buf);

private:
    CUtlBuffer *UpdateRecordingParams(); //called every game frame after entities think and update
    void Reset()
    {
        m_nRecordingTicks = 0;
    }
    bool m_bIsRecording;
    CBasePlayer *m_player;

    char* recordingPath = "recordings";
    char tempRecordingName[BUFSIZELOCL];
    int m_nRecordingTicks;

    FileHandle_t fh;
    CUtlBuffer *buf;
};

extern CMomentumReplaySystem *g_ReplaySystem;

#endif //MOM_REPLAY_H