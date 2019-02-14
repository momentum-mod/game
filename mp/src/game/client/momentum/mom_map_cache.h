#pragma once

#include "mom_shareddefs.h"
#include "steam/isteamhttp.h"
#include "IMapList.h"

enum APIModelSource
{
    MODEL_FROM_DISK = 0,
    MODEL_FROM_LIBRARY_API_CALL,
    MODEL_FROM_SEARCH_API_CALL,
    MODEL_FROM_FAVORITES_API_CALL,
    MODEL_FROM_INFO_API_CALL,
};

abstract_class APIModel
{
public:
    virtual ~APIModel() = default;
    APIModel() : m_bValid(false), m_bUpdated(true), m_eSource(MODEL_FROM_DISK) {}
    bool m_bValid, m_bUpdated;
    APIModelSource m_eSource;
    virtual void FromKV(KeyValues *pKv) = 0;
    virtual void ToKV(KeyValues *pKv) const = 0;
};

struct User : APIModel
{
    uint64 m_uID;
    char m_szAlias[MAX_PLAYER_NAME_LENGTH];
    User() : m_uID(0)
    {
        m_szAlias[0] = '\0';
    }

    void FromKV(KeyValues* pKv) OVERRIDE;
    void ToKV(KeyValues* pKv) const OVERRIDE;
    User& operator=(const User& src);
    bool operator==(const User &other) const { return m_uID == other.m_uID && FStrEq(m_szAlias, other.m_szAlias); }
};

struct MapInfo : APIModel
{
    char m_szDescription[1001];
    int m_iNumBonuses;
    int m_iNumZones;
    bool m_bIsLinear;
    int m_iDifficulty;
    char m_szCreationDate[32];
    MapInfo() : m_iNumBonuses(0), m_iNumZones(0), m_bIsLinear(false), m_iDifficulty(0)
    {
        m_szDescription[0] = '\0';
        m_szCreationDate[0] = '\0';
    }

    void FromKV(KeyValues *pKv) OVERRIDE;
    void ToKV(KeyValues* pKv) const OVERRIDE;
    MapInfo& operator=(const MapInfo& other);
    bool operator==(const MapInfo &other) const;
};

struct MapImage : APIModel
{
    uint32 m_uID;
    char m_szURLSmall[256], m_szURLMedium[256], m_szURLLarge[256];
    char m_szLastUpdatedDate[32];
    MapImage()
    {
        m_uID = 0;
        m_szURLSmall[0] = '\0';
        m_szURLMedium[0] = '\0';
        m_szURLLarge[0] = '\0';
        m_szLastUpdatedDate[0] = '\0';
    }

    void FromKV(KeyValues* pKv) OVERRIDE;
    void ToKV(KeyValues* pKv) const OVERRIDE;
    bool operator==(const MapImage &other) const
    {
        return m_uID == other.m_uID;
    }

    MapImage& operator=(const MapImage& other);
};

struct MapCredit : APIModel
{
    uint32 m_uID;
    MAP_CREDIT_TYPE m_eType;
    User m_User;
    MapCredit() : m_uID(0), m_eType(CREDIT_UNKNOWN) {}

    void FromKV(KeyValues* pKv) OVERRIDE;
    void ToKV(KeyValues* pKv) const OVERRIDE;
    bool operator==(const MapCredit& other) const;
    MapCredit& operator=(const MapCredit& other);
};

struct Run : APIModel
{
    uint64 m_uID;
    bool m_bIsPersonalBest;
    float m_fTickRate;
    char m_szDateAchieved[32]; // ISO date
    float m_fTime; // In seconds
    uint32 m_uFlags;
    char m_szDownloadURL[256];
    char m_szFileHash[41];

    Run(): m_uID(0), m_bIsPersonalBest(false), m_fTickRate(0.0f), m_fTime(.0f), m_uFlags(0)
    {
        m_szDownloadURL[0] = '\0';
        m_szFileHash[0] = '\0';
        m_szDateAchieved[0] = '\0';
    }

    void FromKV(KeyValues* pKv) OVERRIDE;
    void ToKV(KeyValues* pKv) const OVERRIDE;
    bool operator==(const Run& other) const;
    Run& operator=(const Run& other);
};

struct MapRank : APIModel
{
    uint32 m_iRank;
    uint32 m_iRankXP;

    Run m_Run;
    User m_User;

    MapRank() : m_iRank(0), m_iRankXP(0) {}
    bool NeedsUpdate() const { return m_bUpdated || m_Run.m_bUpdated; }
    void ResetUpdate() { m_bUpdated = m_Run.m_bUpdated = false; }
    void FromKV(KeyValues* pKv) OVERRIDE;
    void ToKV(KeyValues* pKv) const OVERRIDE;
    bool operator==(const MapRank& other) const;
    MapRank& operator=(const MapRank& other);
};

struct MapData : APIModel
{
    char m_szLastUpdated[32]; // ISO date, from the site
    bool m_bInFavorites;
    bool m_bInLibrary;
    char m_szPath[MAX_PATH];

    uint32 m_uID;
    char m_szMapName[MAX_MAP_NAME];
    char m_szHash[41];
    GAME_MODE m_eType;
    MAP_UPLOAD_STATUS m_eMapStatus;
    char m_szDownloadURL[256];

    User m_Submitter;
    MapInfo m_Info;
    MapRank m_PersonalBest; // User's rank on a map, if they have one
    MapRank m_WorldRecord; // The world record for the map
    CUtlVector<MapCredit> m_vecCredits;
    MapImage m_Thumbnail;

    // Internal
    bool m_bMapUpdated;

    MapData();
    MapData(const MapData& src);
    bool WasUpdated() const;
    void SendDataUpdate();
    void SendMapFileUpdate();
    void ResetUpdate();
    bool GetCreditString(CUtlString *pOut, MAP_CREDIT_TYPE creditType);
    void FromKV(KeyValues* pMap) OVERRIDE;
    void ToKV(KeyValues* pKv) const OVERRIDE;
    MapData& operator=(const MapData& src);
    bool operator==(const MapData& other) const;
};

class CMapCache : public CAutoGameSystem, public CGameEventListener
{
public:
    CMapCache();

    bool PlayMap(uint32 uID);

    bool AddMapToLibrary(uint32 uID);
    bool RemoveMapFromLibrary(uint32 uID);
    bool AddMapToFavorites(uint32 uID);
    bool RemoveMapFromFavorites(uint32 uID);

    void FireGameEvent(IGameEvent* event) OVERRIDE;

    MapData *GetCurrentMapData() const { return m_pCurrentMapData; }
    uint32 GetCurrentMapID() const { return m_pCurrentMapData ? m_pCurrentMapData->m_uID : 0; }
    MapData *GetMapDataByID(uint32 uMapID);

    void GetMapList(CUtlVector<MapData*> &vecMaps, MapListType_e type);
    bool AddMapsToCache(KeyValues *pData, APIModelSource source);
    void AddMapToCache(KeyValues *pMap, APIModelSource source);
    void FireMapCacheUpdateEvent(APIModelSource source);

    bool UpdateMapInfo(uint32 uMapID);
    uint32 GetUpdateIntervalForMap(MapData *pData);

protected:
    void PostInit() OVERRIDE;
    void LevelInitPreEntity() OVERRIDE;
    void LevelShutdownPostEntity() OVERRIDE;
    void Shutdown() OVERRIDE;

    void LoadMapCacheFromDisk();
    void SaveMapCacheToDisk();

    void SetMapGamemode();

    // HTTP callbacks
    void OnFetchPlayerMapLibrary(KeyValues *pKv);
    void OnFetchPlayerMapFavorites(KeyValues *pKv);
    void OnFetchMapInfo(KeyValues *pKv);

    void OnMapAddedToLibrary(KeyValues *pKv);
    void OnMapRemovedFromLibrary(KeyValues *pKv);
    void OnMapAddedToFavorites(KeyValues *pKv);
    void OnMapRemovedFromFavorites(KeyValues *pKv);

    // Map downloading
    void StartMapDownload(KeyValues *pKvHeader);
    void MapDownloadProgress(KeyValues *pKvProgress);
    void FinishMapDownload(KeyValues *pKvComplete);
private:

    void UpdateFetchedMaps(KeyValues *pKv, bool bIsLibrary);
    void ToggleMapLibraryOrFavorite(KeyValues *pKv, bool bIsLibrary, bool bAdded);

    MapData *m_pCurrentMapData;

    CUtlDict<uint32> m_dictMapNames;
    CUtlMap<uint32, MapData> m_mapMapCache;

    CUtlMap<HTTPRequestHandle, uint32> m_mapFileDownloads;
};

extern CMapCache* g_pMapCache;