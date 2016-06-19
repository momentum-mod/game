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
    virtual ~CMomReplayV1() override;

public:
    virtual uint8 GetVersion() override { return 1; }
    virtual CMomRunStats* GetRunStats() override;
    virtual int32 GetFrameCount() override;
    virtual CReplayFrame* GetFrame(int32 index) override;
    virtual void AddFrame(const CReplayFrame& frame) override;
    virtual bool SetFrame(int32 index, const CReplayFrame& frame) override;
    virtual CMomRunStats* CreateRunStats(uint8 stages) override;
    virtual void RemoveFrames(int num) override;
    virtual void Start(bool firstperson) override;

public:
    virtual void Serialize(CBinaryWriter* writer) override;

private:
    void Deserialize(CBinaryReader* reader);

protected:
    CMomRunStats* m_pRunStats;
    CUtlVector<CReplayFrame> m_rgFrames;
};
