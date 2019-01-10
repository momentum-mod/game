#pragma once

#include "mom_shareddefs.h"

struct User
{
    uint64 m_uID;
    char m_szAlias[MAX_PLAYER_NAME_LENGTH];
    User() : m_uID(0)
    {
        m_szAlias[0] = '\0';
    }

    void FromKV(KeyValues* pKv);

    void ToKV(KeyValues* pKv) const;

    User& operator=(const User& src);
};

struct MapInfo
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

    void FromKV(KeyValues* pKv, bool bAPI);
    void ToKV(KeyValues* pKv) const;
    MapInfo& operator=(const MapInfo& other);
};

struct MapImage
{
    uint32 m_uID;
    char m_szURL[256];
    char m_szHash[41];
    MapImage()
    {
        m_uID = 0;
        m_szURL[0] = '\0';
        m_szHash[0] = '\0';
    }

    void FromKV(KeyValues* pKv);

    void ToKV(KeyValues* pKv) const;

    bool operator==(const MapImage &other) const
    {
        return m_uID == other.m_uID;
    }

    MapImage& operator=(const MapImage& other);
};

struct MapGallery
{
    MapImage m_Thumbnail;
    CUtlVector<MapImage> m_vecExtraImages;
    MapGallery() {}
    MapGallery(const MapGallery& other);

    void FromKV(KeyValues* pKv, bool bAPI);

    void ToKV(KeyValues* pKv) const;

    MapGallery& operator=(const MapGallery& src);
};

struct MapCredit
{
    uint32 m_uID;
    MAP_CREDIT_TYPE m_eType;
    User m_User;
    MapCredit() : m_uID(0), m_eType(CREDIT_UNKNOWN) {}

    void FromKV(KeyValues* pKv);
    void ToKV(KeyValues* pKv) const;
    bool operator==(const MapCredit& other) const;
    MapCredit& operator=(const MapCredit& other);
};

struct MapData
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
    CUtlVector<MapCredit> m_vecCredits;
    MapGallery m_Gallery;

    MapData();
    MapData(const MapData& src);
    void LoadFromKV(KeyValues* pMap, bool bAPI);
    void ToKV(KeyValues* pKv) const;
    MapData& operator=(const MapData& src);
};

class CMapCache : public CAutoGameSystem, public CGameEventListener
{
public:
    CMapCache();

    void OnPlayMap(const char *pMapName);

    void OnPlayerMapLibrary(KeyValues *pKv);

    void FireGameEvent(IGameEvent* event) OVERRIDE;

    MapData *GetCurrentMapData() const { return m_pCurrentMapData; }
    uint32 GetCurrentMapID() const { return m_pCurrentMapData ? m_pCurrentMapData->m_uID : 0; }

protected:
    void PostInit() OVERRIDE;
    void SetMapGamemode();
    void LevelInitPreEntity() OVERRIDE;
    void LevelShutdownPostEntity() OVERRIDE;
    void Shutdown() OVERRIDE;

private:
    MapData *m_pCurrentMapData;

    CUtlDict<uint32> m_dictMapNames;
    CUtlMap<uint32, MapData> m_mapMapCache;
    KeyValues *m_pMapData;
};

extern CMapCache* g_pMapCache;