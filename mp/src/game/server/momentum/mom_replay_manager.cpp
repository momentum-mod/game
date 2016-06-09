#include "cbase.h"
#include "mom_replay_manager.h"
#include "filesystem.h"
#include "mom_replay_v1.h"

#define REPLAY_MAGIC_LE 0x524D4F4D
#define REPLAY_MAGIC_BE 0x4D4F4D52

CUtlMap<uint8, CMomReplayManager::CReplayCreatorBase*> CMomReplayManager::m_mapCreators;
bool CMomReplayManager::m_bDummy = CMomReplayManager::RegisterCreators();

//////////////////////////////////////////////////////////////////////////

bool CMomReplayManager::RegisterCreators()
{
	SetDefLessFunc(CMomReplayManager::m_mapCreators);

	// Register any new replay versions here.
	m_mapCreators.Insert(1, new CMomReplayManager::CReplayCreator<CMomReplayV1>());

	return true;
}

//////////////////////////////////////////////////////////////////////////

CMomReplayManager::CMomReplayManager() :
	m_pCurrentReplay(nullptr),
	m_bRecording(false),
	m_bPlayingBack(false),
	m_ucCurrentVersion(0)
{
	// DO NOT FORGET to set the latest replay version here.
	// This could be automated but CUltMap is a piece of work.
	m_ucCurrentVersion = 1;
}

CMomReplayManager::~CMomReplayManager()
{
	if (m_pCurrentReplay)
		delete m_pCurrentReplay;
}

CMomReplayBase* CMomReplayManager::StartRecording()
{
	if (PlayingBack())
		return nullptr;

	if (Recording())
		return m_pCurrentReplay;

	Log("Started recording a replay...\n");

	m_bRecording = true;

	m_pCurrentReplay = m_mapCreators.Element(m_mapCreators.Find(m_ucCurrentVersion))->CreateReplay();
	return m_pCurrentReplay;
}

void CMomReplayManager::StopRecording()
{
	if (!Recording())
		return;

	Log("Stopped recording a replay.\n");

	m_bRecording = false;

	delete m_pCurrentReplay;
	m_pCurrentReplay = nullptr;
}

CMomReplayBase* CMomReplayManager::LoadReplay(const char* path, const char* pathID)
{
	if (Recording())
		return nullptr;

	if (PlayingBack())
		return m_pCurrentReplay;

	Log("Loading a replay from '%s'...\n", path);

	auto file = filesystem->Open(path, "r+b", pathID);

	if (!file)
		return nullptr;

	CBinaryReader reader(file);

	uint32 magic = reader.ReadUInt32();

	if (magic != REPLAY_MAGIC_LE && magic != REPLAY_MAGIC_BE)
	{
		filesystem->Close(file);
		return nullptr;
	}

	if (magic == REPLAY_MAGIC_BE)
		reader.ShouldFlipEndianness(true);

	uint8 version = reader.ReadUInt8();

	if (m_mapCreators.Find(version) == m_mapCreators.InvalidIndex())
	{
		filesystem->Close(file);
		return nullptr;
	}

	Log("Loading replay '%s' of version '%d'...\n", path, version);

	// TODO (OrfeasZ): Verify that replay parsing was successful.
	m_bPlayingBack = true;
	m_pCurrentReplay = m_mapCreators.Element(m_mapCreators.Find(version))->LoadReplay(&reader);

	filesystem->Close(file);

	Log("Successfully loaded replay.\n");

	return m_pCurrentReplay;
}

bool CMomReplayManager::StoreReplay(const char* path, const char* pathID)
{
	if (!m_pCurrentReplay)
		return false;

	auto file = filesystem->Open(path, "w+b", pathID);

	if (!file)
		return false;

	Log("Storing replay of version '%d' to '%s'...\n", m_pCurrentReplay->GetVersion(), path);

	CBinaryWriter writer(file);

	writer.WriteUInt32(REPLAY_MAGIC_LE);
	writer.WriteUInt8(m_pCurrentReplay->GetVersion());
	m_pCurrentReplay->Serialize(&writer);

	filesystem->Close(file);

	return true;
}

void CMomReplayManager::StopPlayback()
{
	if (!PlayingBack())
		return;

	Log("Stopping replay playback.\n");

	m_bPlayingBack = false;

	delete m_pCurrentReplay;
	m_pCurrentReplay = nullptr;
}