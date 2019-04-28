#include "cbase.h"
#include "mom_replay_versions.h"

#ifdef GAME_DLL
#include "momentum/mom_replay_entity.h"
#include "momentum/mom_timer.h"
#endif

#include "tier0/memdbgon.h"

CMomReplayV1::CMomReplayV1(CUtlBuffer &reader, bool bFull)
    : CMomReplayBase(CReplayHeader(reader), bFull), m_pRunStats(nullptr)
{
    Deserialize(reader, bFull);
}

CMomReplayV1::CMomReplayV1() : CMomReplayBase(CReplayHeader(), true), m_pRunStats(nullptr) {}

CMomReplayV1::~CMomReplayV1()
{
    if (m_pRunStats)
    {
        delete m_pRunStats;
        m_pRunStats = nullptr;
    }
    m_rgFrames.Purge();
}

CMomRunStats *CMomReplayV1::GetRunStats() { return m_pRunStats; }

int32 CMomReplayV1::GetFrameCount() { return m_rgFrames.Count(); }

CReplayFrame *CMomReplayV1::GetFrame(int32 index)
{
    if (index >= m_rgFrames.Count() || index < 0)
        return nullptr;

    return &m_rgFrames[index];
}

void CMomReplayV1::AddFrame(const CReplayFrame &frame) { m_rgFrames.AddToTail(frame); }

bool CMomReplayV1::SetFrame(int32 index, const CReplayFrame &frame)
{
    if (index >= m_rgFrames.Count() || index < 0)
        return false;

    m_rgFrames[index] = frame;
    return true;
}

CMomRunStats *CMomReplayV1::CreateRunStats(uint8 zones)
{
    if (m_pRunStats != nullptr)
        delete m_pRunStats;

    m_pRunStats = new CMomRunStats(zones);
    return m_pRunStats;
}

void CMomReplayV1::RemoveFrames(int num) { m_rgFrames.RemoveMultipleFromHead(num); }

void CMomReplayV1::Serialize(CUtlBuffer &writer)
{
    // Write the header.
    m_rhHeader.Serialize(writer);

    // Write the run stats (if there are any).
    writer.PutUnsignedChar(m_pRunStats != nullptr);

    if (m_pRunStats != nullptr)
        m_pRunStats->Serialize(writer);

    // Write the frames.
    writer.PutInt(m_rgFrames.Count());

    for (int32 i = 0; i < m_rgFrames.Count(); ++i)
        m_rgFrames[i].Serialize(writer);
}

// bFull is defined by a replay being played back vs. a replay being loaded for comparisons
void CMomReplayV1::Deserialize(CUtlBuffer &reader, bool bFull)
{
    // Read the run stats (if there are any).
    if (reader.GetUnsignedChar())
    {
        m_pRunStats = new CMomRunStats(reader);
    }

    if (bFull)
    {
        // Read the count of frames in the replay
        // and ensure we have the capacity to store them.
        int32 frameCount = reader.GetInt();

        if (frameCount <= 0)
            return;

        // And read all the frames.
        for (int32 i = 0; i < frameCount; ++i)
            m_rgFrames.AddToTail(CReplayFrame(reader));
    }
}