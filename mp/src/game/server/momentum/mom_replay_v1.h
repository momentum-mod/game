#pragma once

#include "cbase.h"
#include "mom_replay_base.h"
#include "mom_replay_entity.h"

class CMomReplayV1 :
    public CMomReplayBase
{
public:
    CMomReplayV1();
    CMomReplayV1(CBinaryReader* reader);
    ~CMomReplayV1() override;

public:
    uint8 GetVersion() override { return 1; }
    CMomRunStats* GetRunStats() override;
    int32 GetFrameCount() override;
    CReplayFrame* GetFrame(int32 index) override;
    void AddFrame(const CReplayFrame& frame) override;
    bool SetFrame(int32 index, const CReplayFrame& frame) override;
    CMomRunStats* CreateRunStats(uint8 stages) override;
    void RemoveFrames(int num) override;
    void Start(bool firstperson) override;

public:
    void Serialize(CBinaryWriter* writer) override;

private:
    void Deserialize(CBinaryReader* reader);

protected:
    CMomRunStats* m_pRunStats;
    CUtlVector<CReplayFrame> m_rgFrames;
};
