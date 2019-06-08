#pragma once

#include "mom_shareddefs.h"

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
    APIModel();
    virtual ~APIModel() {}

    bool m_bValid, m_bUpdated;
    APIModelSource m_eSource;
    virtual void FromKV(KeyValues *pKv) = 0;
    virtual void ToKV(KeyValues *pKv) const = 0;
};

struct User : APIModel
{
    uint32 m_uMainID;
    uint64 m_uSteamID;
    char m_szAlias[MAX_PLAYER_NAME_LENGTH];
    User();

    void FromKV(KeyValues* pKv) OVERRIDE;
    void ToKV(KeyValues* pKv) const OVERRIDE;
    User& operator=(const User& src);
    bool operator==(const User &other) const;
};

struct MapInfo : APIModel
{
    char m_szDescription[1001];
    int m_iNumTracks;
    char m_szCreationDate[32];
    MapInfo();

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
    MapImage();

    void FromKV(KeyValues* pKv) OVERRIDE;
    void ToKV(KeyValues* pKv) const OVERRIDE;
    bool operator==(const MapImage &other) const;
    MapImage& operator=(const MapImage& other);
};

struct MapCredit : APIModel
{
    uint32 m_uID;
    MapCreditType_t m_eType;
    User m_User;
    MapCredit();

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
    Run();

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
    MapRank();

    bool NeedsUpdate() const;
    void ResetUpdate();
    void FromKV(KeyValues* pKv) OVERRIDE;
    void ToKV(KeyValues* pKv) const OVERRIDE;
    bool operator==(const MapRank& other) const;
    MapRank& operator=(const MapRank& other);
};

struct MapTrack : APIModel
{
    uint8 m_iTrackNum;
    uint8 m_iNumZones;
    bool m_bIsLinear;
    uint8 m_iDifficulty;
    MapTrack();

    void FromKV(KeyValues *pKv) OVERRIDE;
    void ToKV(KeyValues *pKv) const OVERRIDE;
    bool operator==(const MapTrack &other) const;
    MapTrack &operator=(const MapTrack &other);
};

struct MapData : APIModel
{
    char m_szLastUpdated[32]; // ISO date, from the site
    char m_szCreatedAt[32]; // ISO date, from the site
    bool m_bInFavorites;
    bool m_bInLibrary;

    uint32 m_uID;
    char m_szMapName[MAX_MAP_NAME];
    char m_szHash[41];
    GameMode_t m_eType;
    MapUploadStatus_t m_eMapStatus;
    char m_szDownloadURL[256];

    User m_Submitter;
    MapInfo m_Info;
    MapTrack m_MainTrack;
    MapRank m_PersonalBest; // User's rank on a map, if they have one
    MapRank m_WorldRecord; // The world record for the map
    CUtlVector<MapCredit> m_vecCredits;
    CUtlVector<MapImage> m_vecImages;
    MapImage m_Thumbnail;

    // Internal
    bool m_bMapFileExists;
    bool m_bMapFileNeedsUpdate;
    time_t m_tLastPlayed;
    MapData();
    MapData(const MapData& src);

    bool WasUpdated() const;
    void SendDataUpdate();
    void ResetUpdate();
    bool GetCreditString(CUtlString *pOut, MapCreditType_t creditType);
    void DeleteMapFile();
    void FromKV(KeyValues* pMap) OVERRIDE;
    void ToKV(KeyValues* pKv) const OVERRIDE;
    MapData& operator=(const MapData& src);
    bool operator==(const MapData& other) const;
};