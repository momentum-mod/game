#pragma once

#include "mom_shareddefs.h"

abstract_class APIModel 
{
public:
    virtual ~APIModel() = default;
    APIModel() : m_bValid(false), m_bFromAPI(true) {}
    bool m_bValid, m_bFromAPI;
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
};

struct MapInfo : APIModel
{
    char m_szDescription[1001];
    int m_iNumBonuses;
    int m_iNumZones;
    bool m_bIsLinear;
    int m_iDifficulty;
    time_t m_tCreationDate;
    MapInfo() : m_iNumBonuses(0), m_iNumZones(0), m_bIsLinear(false), m_iDifficulty(0), m_tCreationDate(0)
    {
        m_szDescription[0] = '\0';
    }

    void FromKV(KeyValues *pKv) OVERRIDE;
    void ToKV(KeyValues* pKv) const OVERRIDE;
    MapInfo& operator=(const MapInfo& other);
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

struct MapGallery : APIModel
{
    MapImage m_Thumbnail;
    CUtlVector<MapImage> m_vecExtraImages;
    MapGallery() {}
    MapGallery(const MapGallery& other);

    void FromKV(KeyValues* pKv) OVERRIDE;
    void ToKV(KeyValues* pKv) const OVERRIDE;
    MapGallery& operator=(const MapGallery& src);
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
    // dateAchieved : DATE
    float m_fTime; // In seconds
    uint32 m_uFlags;
    char m_szDownloadURL[256];
    char m_szFileHash[41];

    Run(): m_uID(0), m_bIsPersonalBest(false), m_fTickRate(0.0f), m_fTime(.0f), m_uFlags(0)
    {
        m_szDownloadURL[0] = '\0';
        m_szFileHash[0] = '\0';
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

    MapRank() : m_iRank(0), m_iRankXP(0) {}

    void FromKV(KeyValues* pKv) OVERRIDE;
    void ToKV(KeyValues* pKv) const OVERRIDE;
    bool operator==(const MapRank& other) const;
    MapRank& operator=(const MapRank& other);
};

struct MapData : APIModel
{
    time_t m_tLastUpdated;
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

    MapData();
    MapData(const MapData& src);
    void FromKV(KeyValues* pMap) OVERRIDE;
    void ToKV(KeyValues* pKv) const OVERRIDE;
    MapData& operator=(const MapData& src);
    bool operator==(const MapData& other) const;
};

class CMapCache : public CAutoGameSystem, public CGameEventListener
{
public:
    CMapCache();

    void OnPlayMap(const char *pMapName);


    void FireGameEvent(IGameEvent* event) OVERRIDE;

    MapData *GetCurrentMapData() const { return m_pCurrentMapData; }
    uint32 GetCurrentMapID() const { return m_pCurrentMapData ? m_pCurrentMapData->m_uID : 0; }

    void GetMapLibrary(CUtlVector<MapData*> &vecLibrary);

protected:
    void PostInit() OVERRIDE;
    void LevelInitPreEntity() OVERRIDE;
    void LevelShutdownPostEntity() OVERRIDE;
    void Shutdown() OVERRIDE;

    void SetMapGamemode();
    void OnPlayerMapLibrary(KeyValues *pKv);
private:
    MapData *m_pCurrentMapData;

    CUtlDict<uint32> m_dictMapNames;
    CUtlMap<uint32, MapData> m_mapMapCache;
    KeyValues *m_pMapData;
};

extern CMapCache* g_pMapCache;