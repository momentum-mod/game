#include "cbase.h"
#include "mom_replay_v1.h"

CMomReplayV1::CMomReplayV1(CBinaryReader* reader) :
	CMomReplayBase(CReplayHeader(reader)),
	m_pRunStats(nullptr)
{
	Deserialize(reader);
}

CMomReplayV1::CMomReplayV1() :
	CMomReplayBase(CReplayHeader()),
	m_pRunStats(nullptr)
{
}

CMomReplayV1::~CMomReplayV1()
{
	if (m_pRunStats)
	{
		delete m_pRunStats;
		m_pRunStats = nullptr;
	}
}

CMomRunStats* CMomReplayV1::GetRunStats()
{
	return m_pRunStats;
}

int32 CMomReplayV1::GetFrameCount()
{
	return m_rgFrames.Size();
}

CReplayFrame* CMomReplayV1::GetFrame(int32 index)
{
	if (index >= m_rgFrames.Size() || index < 0)
		return nullptr;

	return &m_rgFrames[index];
}

void CMomReplayV1::AddFrame(const CReplayFrame& frame)
{
	m_rgFrames.AddToTail(frame);
}

bool CMomReplayV1::SetFrame(int32 index, const CReplayFrame& frame)
{
	if (index >= m_rgFrames.Size() || index < 0)
		return false;

	m_rgFrames[index] = frame;
	return true;
}

CMomRunStats* CMomReplayV1::CreateRunStats(uint8 stages)
{
	if (m_pRunStats != nullptr)
		delete m_pRunStats;

	m_pRunStats = new CMomRunStats(stages);
	return m_pRunStats;
}

void CMomReplayV1::Serialize(CBinaryWriter* writer)
{
	// Write the run stats (if there are any).
	writer->WriteBool(m_pRunStats != nullptr);

	if (m_pRunStats != nullptr)
		m_pRunStats->Serialize(writer);

	// Write the frames.
	writer->WriteInt32(m_rgFrames.Size());

	for (int32 i = 0; i < m_rgFrames.Size(); ++i)
		m_rgFrames[i].Serialize(writer);
}

void CMomReplayV1::Deserialize(CBinaryReader* reader)
{
	// Read the run stats (if there are any).
	if (reader->ReadBool())
		m_pRunStats = new CMomRunStats(reader);

	// Read the count of frames in the replay 
	// and ensure we have the capacity to store them.
	int32 frameCount = reader->ReadInt32();

	if (frameCount <= 0)
		return;

	m_rgFrames.EnsureCapacity(frameCount);

	// And read all the frames.
	for (int32 i = 0; i < frameCount; ++i)
		m_rgFrames.AddToTail(CReplayFrame(reader));
}