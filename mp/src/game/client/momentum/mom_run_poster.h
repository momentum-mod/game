#pragma once

#include "GameEventListener.h"
#include "igamesystem.h"
#include "mom_shareddefs.h"

#define ENABLE_STEAM_LEADERBOARDS 0

#if ENABLE_STEAM_LEADERBOARDS
#include "steam/steam_api.h"
#endif

class CRunPoster : public CGameEventListener, public CAutoGameSystemPerFrame
{
  public:
    CRunPoster();
    ~CRunPoster();

    void PostInit() OVERRIDE;
    void LevelInitPostEntity() OVERRIDE;
    void LevelShutdownPostEntity() OVERRIDE;

    void PreRender() OVERRIDE;

    void FireGameEvent(IGameEvent *pEvent) OVERRIDE;

    // Callbacks
    void InvalidateSessionCallback(KeyValues *pKv);
    void CreateSessionCallback(KeyValues *pKv);
    void UpdateSessionCallback(KeyValues *pKv);
    void EndSessionCallback(KeyValues *pKv);

private:
    RunSubmitState_t ShouldSubmitRun();
    bool CheckCurrentMap();
    void ResetSession();
    bool m_bIsMappingMode;
    uint64 m_uRunSessionID;
    int m_iZoneEnterTicks[MAX_ZONES];

#if ENABLE_STEAM_LEADERBOARDS
public:
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
#endif
};

extern CRunPoster *g_pRunPoster;