#pragma once

#include "cbase.h"

#include "GameEventListener.h"
#include "steam/steam_api.h"

class CRunPoster : public CGameEventListener, public CAutoGameSystem
{
  public:
    CRunPoster();
    ~CRunPoster();

    void PostInit() OVERRIDE;
    void LevelInitPostEntity() OVERRIDE;
    void LevelShutdownPreClearSteamAPIContext() OVERRIDE;

    void FireGameEvent(IGameEvent *pEvent) OVERRIDE;

    SteamLeaderboard_t m_hCurrentLeaderboard;
    CCallResult<CRunPoster, LeaderboardFindResult_t> m_cLeaderboardFindResult;
    void OnLeaderboardFind(LeaderboardFindResult_t *pResult, bool bIOFailure);

    CCallResult<CRunPoster, LeaderboardScoreUploaded_t> m_cLeaderboardScoreUploaded;
    void OnLeaderboardScoreUploaded(LeaderboardScoreUploaded_t *pResult, bool bIOFailure);

    CCallResult<CRunPoster, LeaderboardUGCSet_t> m_cLeaderboardUGCSet;
    void OnLeaderboardUGCSet(LeaderboardUGCSet_t *pResult, bool bIOFailure);

    CCallResult<CRunPoster, RemoteStorageFileWriteAsyncComplete_t> m_cFileUploaded;
    void OnFileUploaded(RemoteStorageFileWriteAsyncComplete_t* pResult, bool bIOFailure);

    CCallResult<CRunPoster, RemoteStorageFileShareResult_t> m_cFileShared;
    void OnFileShared(RemoteStorageFileShareResult_t *pResult, bool bIOFailure);

private:
    char m_szFileName[MAX_PATH];
    char m_szFilePath[MAX_PATH];
};