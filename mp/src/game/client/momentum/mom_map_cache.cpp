#include "cbase.h"

#include <ctime>

#include "mom_map_cache.h"
#include "mom_shareddefs.h"
#include "mom_api_requests.h"
#include "util/mom_util.h"

#include "filesystem.h"

#include "tier0/memdbgon.h"

#define UPDATE_INTERVAL 3600 // 1 hour between updates
#define MAP_CACHE_FILE_NAME "map_cache.dat"

struct User
{
    uint64 m_uID;
    char m_szAlias[MAX_PLAYER_NAME_LENGTH];
    User(): m_uID(0)
    {
        m_szAlias[0] = '\0';
    }
    void FromKV(KeyValues *pKv)
    {
        m_uID = pKv->GetUint64("id");
        Q_strncpy(m_szAlias, pKv->GetString("alias"), sizeof(m_szAlias));
    }
    void ToKV(KeyValues *pKv) const
    {
        pKv->SetUint64("id", m_uID);
        pKv->SetString("alias", m_szAlias);
    }
    User& operator=(const User &src)
    {
        m_uID = src.m_uID;
        Q_strncpy(m_szAlias, src.m_szAlias, sizeof(m_szAlias));
        return *this;
    }
};

struct MapInfo
{
    char m_szDescription[1001];
    int m_iNumBonuses;
    int m_iNumZones;
    bool m_bIsLinear;
    int m_iDifficulty;
    time_t m_tCreationDate;
    MapInfo(): m_iNumBonuses(0), m_iNumZones(0), m_bIsLinear(false), m_iDifficulty(0), m_tCreationDate(0)
    {
        m_szDescription[0] = '\0';
    }

    void FromKV(KeyValues *pKv, bool bAPI)
    {
        Q_strncpy(m_szDescription, pKv->GetString("description"), sizeof(m_szDescription));
        m_iNumBonuses = pKv->GetInt("numBonuses");
        m_iNumZones = pKv->GetInt("numZones");
        m_bIsLinear = pKv->GetBool("isLinear");
        m_iDifficulty = pKv->GetInt("difficulty");
        if (bAPI)
            g_pMomentumUtil->ISODateToTimeT(pKv->GetString("creationDate"), &m_tCreationDate);
        else
            m_tCreationDate = (time_t) pKv->GetUint64("creationDate");
    }
    void ToKV(KeyValues *pKv) const
    {
        pKv->SetString("description", m_szDescription);
        pKv->SetInt("numBonuses", m_iNumBonuses);
        pKv->SetInt("numZones", m_iNumZones);
        pKv->SetInt("difficulty", m_iDifficulty);
        pKv->SetBool("isLinear", m_bIsLinear);
        pKv->SetUint64("creationDate", m_tCreationDate);
    }
    MapInfo& operator=(const MapInfo &other)
    {
        Q_strncpy(m_szDescription, other.m_szDescription, sizeof(m_szDescription));
        m_iNumBonuses = other.m_iNumBonuses;
        m_iNumZones = other.m_iNumZones;
        m_bIsLinear = other.m_bIsLinear;
        m_iDifficulty = other.m_iDifficulty;
        m_tCreationDate = other.m_tCreationDate;
        return *this;
    }
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
    void FromKV(KeyValues *pKv)
    {
        m_uID = pKv->GetInt("id");
        Q_strncpy(m_szURL, pKv->GetString("URL"), sizeof(m_szURL));
        Q_strncpy(m_szHash, pKv->GetString("hash"), sizeof(m_szHash));
    }
    void ToKV(KeyValues *pKv) const
    {
        pKv->SetInt("id", m_uID);
        pKv->SetString("URL", m_szURL);
        pKv->SetString("hash", m_szHash);
    }
    bool operator==(const MapImage &other) const
    {
        return m_uID == other.m_uID;
    }
    MapImage& operator=(const MapImage &other)
    {
        m_uID = other.m_uID;
        Q_strncpy(m_szURL, other.m_szURL, sizeof(m_szURL));
        Q_strncpy(m_szHash, other.m_szHash, sizeof(m_szHash));
        return *this;
    }
};

struct MapGallery
{
    MapImage m_Thumbnail;
    CUtlVector<MapImage> m_vecExtraImages;
    MapGallery(){}
    MapGallery(const MapGallery &other)
    {
        m_Thumbnail = other.m_Thumbnail;
        m_vecExtraImages.AddVectorToTail(other.m_vecExtraImages);
    }
    void FromKV(KeyValues *pKv, bool bAPI)
    {
        KeyValues *pThumbnail = pKv->FindKey("thumbnail");
        if (pThumbnail)
            m_Thumbnail.FromKV(pThumbnail);
        KeyValues *pExtraImages = pKv->FindKey("extraImages");
        if (pExtraImages)
        {
            FOR_EACH_SUBKEY(pExtraImages, pExtraImage)
            {
                MapImage mi;
                mi.FromKV(pExtraImage);

                if (bAPI)
                {
                    uint16 indx = m_vecExtraImages.Find(mi);
                    if (m_vecExtraImages.IsValidIndex(indx))
                    {
                        m_vecExtraImages[indx] = mi;
                        continue;
                    }
                }

                m_vecExtraImages.AddToTail(mi);
            }
        }
    }
    void ToKV(KeyValues *pKv) const
    {
        KeyValues *pThumbnail = new KeyValues("thumbnail");
        m_Thumbnail.ToKV(pThumbnail);
        pKv->AddSubKey(pThumbnail);

        if (!m_vecExtraImages.IsEmpty())
        {
            KeyValues *pExtras = new KeyValues("extraImages");
            FOR_EACH_VEC(m_vecExtraImages, i)
            {
                KeyValues *pExtraImage = pExtras->CreateNewKey();
                m_vecExtraImages[i].ToKV(pExtraImage);
            }
            pKv->AddSubKey(pExtras);
        }
    }
    MapGallery& operator=(const MapGallery &src)
    {
        m_Thumbnail = src.m_Thumbnail;
        m_vecExtraImages.RemoveAll();
        m_vecExtraImages.AddVectorToTail(src.m_vecExtraImages);
        return *this;
    }
};

struct MapCredit
{
    uint32 m_uID;
    MAP_CREDIT_TYPE m_eType;
    User m_User;
    MapCredit(): m_uID(0), m_eType(CREDIT_UNKNOWN) {}

    void FromKV(KeyValues *pKv)
    {
        m_uID = pKv->GetInt("id");
        m_eType = (MAP_CREDIT_TYPE) pKv->GetInt("type", -1);
        KeyValues *pUser = pKv->FindKey("user");
        if (pUser)
        {
            m_User.FromKV(pUser);
        }
    }
    void ToKV(KeyValues *pKv) const
    {
        pKv->SetInt("id", m_uID);
        pKv->SetInt("type", m_eType);
        KeyValues *pUser = new KeyValues("user");
        m_User.ToKV(pUser);
        pKv->AddSubKey(pUser);
    }
    bool operator==(const MapCredit &other) const
    {
        return m_uID == other.m_uID;
    }
    MapCredit& operator=(const MapCredit &other)
    {
        m_uID = other.m_uID;
        m_eType = other.m_eType;
        m_User = other.m_User;
        return *this;
    }
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

    MapData()
    {
        m_tLastUpdated = 0;
        m_bInLibrary = false;
        m_bInFavorites = false;
        m_szPath[0] = '\0';
        m_uID = 0;
        m_szMapName[0] = '\0';
        m_szHash[0] = '\0';
        m_eType = GAMEMODE_UNKNOWN;
        m_eMapStatus = STATUS_UNKNOWN;
        m_szDownloadURL[0] = '\0';
    }

    MapData(const MapData &src) 
    {
        m_uID = src.m_uID;
        m_eType = src.m_eType;
        m_eMapStatus = src.m_eMapStatus;
        Q_strncpy(m_szHash, src.m_szHash, sizeof(m_szHash));
        Q_strncpy(m_szDownloadURL, src.m_szDownloadURL, sizeof(m_szDownloadURL));

        Q_strncpy(m_szMapName, src.m_szMapName, sizeof(m_szMapName));
        m_bInFavorites = src.m_bInFavorites;
        m_bInLibrary = src.m_bInLibrary;
        m_tLastUpdated = src.m_tLastUpdated;

        Q_strncpy(m_szPath, src.m_szPath, sizeof(m_szPath));

        m_Info = src.m_Info;
        m_Submitter = src.m_Submitter;
        m_vecCredits.RemoveAll();
        m_vecCredits.AddMultipleToTail(src.m_vecCredits.Count(), src.m_vecCredits.Base());
        m_Gallery = src.m_Gallery;
    }

    void LoadFromKV(KeyValues *pMap, bool bAPI)
    {
        m_uID = pMap->GetInt("id");
        m_eType = (GAME_MODE) pMap->GetInt("type");
        m_eMapStatus = (MAP_UPLOAD_STATUS) pMap->GetInt("statusFlag", -1);
        Q_strncpy(m_szHash, pMap->GetString("hash"), sizeof(m_szHash));
        Q_strncpy(m_szDownloadURL, pMap->GetString("downloadURL"), sizeof(m_szDownloadURL));

        Q_strncpy(m_szMapName, bAPI ? pMap->GetString("name") : pMap->GetName(), sizeof(m_szMapName));
        m_bInFavorites = bAPI ? pMap->FindKey("favorites") != nullptr : pMap->GetBool("inFavorites");
        m_bInLibrary = bAPI ? true : pMap->GetBool("inLibrary");
        m_tLastUpdated = bAPI ? time(nullptr) : (time_t) pMap->GetUint64("lastUpdated");

        if (!bAPI)
            Q_strncpy(m_szPath, pMap->GetString("path"), sizeof(m_szPath));

        KeyValues *pInfo = pMap->FindKey("info");
        if (pInfo)
            m_Info.FromKV(pInfo, bAPI);
        KeyValues *pSubmitter = pMap->FindKey("submitter");
        if (pSubmitter)
            m_Submitter.FromKV(pSubmitter);
        KeyValues *pCredits = pMap->FindKey("credits");
        if (pCredits)
        {
            FOR_EACH_SUBKEY(pCredits, pCredit)
            {
                MapCredit mc;
                mc.FromKV(pCredit);
                if (bAPI)
                {
                    uint16 indx = m_vecCredits.Find(mc);
                    if (m_vecCredits.IsValidIndex(indx))
                    {
                        m_vecCredits[indx] = mc;
                        continue;
                    }
                }

                m_vecCredits.AddToTail(mc);
            }
        }

        KeyValues *pGallery = pMap->FindKey("gallery");
        if (pGallery)
            m_Gallery.FromKV(pGallery, bAPI);
    }

    void ToKV(KeyValues *pKv) const
    {
        pKv->SetName(m_szMapName);
        pKv->SetInt("id", m_uID);
        pKv->SetInt("type", m_eType);
        pKv->SetInt("statusFlag", m_eMapStatus);
        pKv->SetString("hash", m_szHash);
        pKv->SetString("downloadURL", m_szDownloadURL);
        pKv->SetBool("inFavorites", m_bInFavorites);
        pKv->SetBool("inLibrary", m_bInLibrary);
        pKv->SetUint64("lastUpdated", m_tLastUpdated);
        pKv->SetString("path", m_szPath);

        KeyValues *pInfo = new KeyValues("info");
        m_Info.ToKV(pInfo);
        pKv->AddSubKey(pInfo);

        KeyValues *pSubmitter = new KeyValues("submitter");
        m_Submitter.ToKV(pSubmitter);
        pKv->AddSubKey(pSubmitter);

        if (!m_vecCredits.IsEmpty())
        {
            KeyValues *pCredits = new KeyValues("credits");
            FOR_EACH_VEC(m_vecCredits, i)
            {
                KeyValues *pCredit = pCredits->CreateNewKey();
                m_vecCredits[i].ToKV(pCredit);
            }
            pKv->AddSubKey(pCredits);
        }

        KeyValues *pGallery = new KeyValues("gallery");
        m_Gallery.ToKV(pGallery);
        pKv->AddSubKey(pGallery);
    }

    MapData& operator=(const MapData &src)
    {
        m_uID = src.m_uID;
        m_eType = src.m_eType;
        m_eMapStatus = src.m_eMapStatus;
        Q_strncpy(m_szHash, src.m_szHash, sizeof(m_szHash));
        Q_strncpy(m_szDownloadURL, src.m_szDownloadURL, sizeof(m_szDownloadURL));

        Q_strncpy(m_szMapName, src.m_szMapName, sizeof(m_szMapName));
        m_bInFavorites = src.m_bInFavorites;
        m_bInLibrary = src.m_bInLibrary;
        m_tLastUpdated = src.m_tLastUpdated;

        Q_strncpy(m_szPath, src.m_szPath, sizeof(m_szPath));

        m_Info = src.m_Info;
        m_Submitter = src.m_Submitter;
        m_vecCredits.RemoveAll();
        m_vecCredits.AddMultipleToTail(src.m_vecCredits.Count(), src.m_vecCredits.Base());
        m_Gallery = src.m_Gallery;

        return *this;
    }
};

/**
 *  "MapCacheData"
 *  {
 *      "<map_name>"
 *      {
 *          "lastUpdated" "<time/epoch>"
 *          "inLibrary" <bool>
 *          "inFavorites" <bool>
 *          "path" "<full path to the map file>"
 *          
 *          "id" "<id>"
 *          "hash" "<hash>"
 *          "type" "<type>"
 *          "statusFlag" "<status>"
 *          "downloadURL" "<downloadURL>"
 *          
 *          "submitter" (user)
 *          {
 *              id
 *              alias
 *          }
 *          
 *          "info"
 *          {
 *              description
 *              numBonuses
 *              numZones
 *              isLinear
 *              difficulty
 *              creationDate
 *          }
 *          "credits"
 *          {
 *              "<array index>"
 *              {
 *                  type
 *                  "user"
 *                  {
 *                      id
 *                      alias
 *                  }
 *              }
 *          }
 *          
 *          "gallery"
 *          {
 *              "thumbnail" 
 *              {
 *                  id
 *                  url
 *                  hash
 *              }
 *              "extraImages"
 *              {
 *                  
 *              }
 *          }
 *      }
 *  }
 */

CMapCache::CMapCache() : CAutoGameSystem("CMapCache"), m_pCurrentMapData(nullptr)
{
    m_pMapData = new KeyValues("MapCacheData");
    SetDefLessFunc(m_mapMapCache);
}

void CMapCache::OnPlayerMapLibrary(KeyValues* pKv)
{
    KeyValues *pData = pKv->FindKey("data");
    KeyValues *pErr = pKv->FindKey("error");

    if (pData)
    {
        int count = pData->GetInt("count");

        if (count)
        {
            KeyValues *pEntries = pData->FindKey("entries");
            FOR_EACH_SUBKEY(pEntries, pEntry)
            {
                KeyValues *pMap = pEntry->FindKey("map");
                if (pMap)
                {
                    MapData d;
                    d.LoadFromKV(pMap, true);

                    uint16 indx = m_mapMapCache.Find(pMap->GetInt("id"));
                    if (m_mapMapCache.IsValidIndex(indx))
                    {
                        // Update it
                        m_mapMapCache[indx] = d;
                    }
                    else
                    {
                        m_dictMapNames.Insert(d.m_szMapName, d.m_uID);
                        m_mapMapCache.Insert(d.m_uID, d);
                    }
                }
            }
        }
    }
    else if (pErr)
    {
        
    }
}

void CMapCache::FireGameEvent(IGameEvent* event)
{
    if (g_pAPIRequests->IsAuthenticated())
    {
        // Regardless, we query the user's map library for map cache updating
        if (g_pAPIRequests->GetUserMapLibrary(UtlMakeDelegate(this, &CMapCache::OnPlayerMapLibrary)))
        {
            DevLog("Requested the library!\n");
        }
        else
        {
            DevLog("Failed to request the library\n");
        }
    }
    else
    {
        DevWarning("We're not authenticated!\n");
    }
}

void CMapCache::PostInit()
{
    ListenForGameEvent("site_auth");

    // Load the cache from disk
    if (m_pMapData->LoadFromFile(g_pFullFileSystem, MAP_CACHE_FILE_NAME, "MOD"))
    {
        FOR_EACH_SUBKEY(m_pMapData, pMap)
        {
            MapData d;
            d.LoadFromKV(pMap, false);

            m_dictMapNames.Insert(d.m_szMapName, d.m_uID);
            m_mapMapCache.Insert(d.m_uID, d);
        }

    }
    else
    {
        DevLog(2, "Map cache file doesn't exist, creating it...");
    }
}

void CMapCache::LevelInitPostEntity()
{
    // Check if the map was updated recently
    const char *pMapName = MapName();
    int dictIndx = m_dictMapNames.Find(pMapName);
    if (m_dictMapNames.IsValidIndex(dictIndx))
    {
        uint32 mapID = m_dictMapNames[dictIndx];
        uint16 mapIndx = m_mapMapCache.Find(mapID);
        if (m_mapMapCache.IsValidIndex(mapIndx))
        {
            m_pCurrentMapData = &m_mapMapCache[mapIndx];

            if (m_pCurrentMapData->m_tLastUpdated)
            {
                if (time(nullptr) - m_pCurrentMapData->m_tLastUpdated > UPDATE_INTERVAL)
                {
                    // Needs update
                    DevLog("The map needs updating!\n");
                }
                else
                {
                    // Is fine
                }
            }
            else
            {
                // Wasn't ever updated (?), needs update
                DevLog("The map needs updating!\n");
            }
        }
    }
}

void CMapCache::LevelShutdownPostEntity()
{
    // MOM_TODO redundant save to disk?
}

void CMapCache::Shutdown()
{
    if (m_pMapData)
    {
        m_pMapData->Clear();

        uint16 indx = m_mapMapCache.FirstInorder();
        while (indx != m_mapMapCache.InvalidIndex())
        {
            KeyValues *pMap = m_pMapData->CreateNewKey();
            m_mapMapCache[indx].ToKV(pMap);

            indx = m_mapMapCache.NextInorder(indx);
        }

        if (!m_pMapData->SaveToFile(g_pFullFileSystem, MAP_CACHE_FILE_NAME, "MOD"))
            DevLog("Failed to log map cache out to file\n");

        m_pMapData->deleteThis();
    }
}

CMapCache s_mapCache;
CMapCache *g_pMapCache = &s_mapCache;