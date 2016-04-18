#ifndef MOM_REPLAY_H
#define MOM_REPLAY_H
#include "cbase.h"
#include "filesystem.h"

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
            UpdateRecordingParams();
            WriteRecordingToFile();
        }
    }
    void BeginRecording(CBasePlayer *pPlayer);
    void StopRecording(CBasePlayer *pPlayer);
    bool IsRecording(CBasePlayer *pPlayer) { return m_bIsRecording; }
    void WriteRecordingToFile();

private:
    void UpdateRecordingParams(); //called every game frame after entities think and update
    void Reset()
    {
        m_nRecordingTicks = 0;
    }
    bool m_bIsRecording;
    CBasePlayer *m_player;
    struct replay_t
    {
        QAngle m_qEyeAngles;
        Vector m_vPlayerOrigin;
        Vector m_vPlayerVelocity;
        int m_nPlayerButtons;
    };
    replay_t m_currentFrame; 

    char* recordingPath = "recordings";
    char tempRecordingName[BUFSIZELOCL];
    int m_nRecordingTicks;
};

extern CMomentumReplaySystem *g_ReplaySystem;

#endif //MOM_REPLAY_H