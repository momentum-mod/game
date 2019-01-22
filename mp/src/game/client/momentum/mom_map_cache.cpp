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

void User::FromKV(KeyValues* pKv)
{
    m_uID = pKv->GetUint64("id");
    Q_strncpy(m_szAlias, pKv->GetString("alias"), sizeof(m_szAlias));
    m_bValid = true;
}

void User::ToKV(KeyValues* pKv) const
{
    pKv->SetUint64("id", m_uID);
    pKv->SetString("alias", m_szAlias);
}

User& User::operator=(const User& src)
{
    m_uID = src.m_uID;
    Q_strncpy(m_szAlias, src.m_szAlias, sizeof(m_szAlias));
    m_bValid = src.m_bValid;
    return *this;
}

void MapInfo::FromKV(KeyValues* pKv)
{
    Q_strncpy(m_szDescription, pKv->GetString("description"), sizeof(m_szDescription));
    m_iNumBonuses = pKv->GetInt("numBonuses");
    m_iNumZones = pKv->GetInt("numZones");
    m_bIsLinear = pKv->GetBool("isLinear");
    m_iDifficulty = pKv->GetInt("difficulty");
    if (m_bFromAPI)
        m_bValid = g_pMomentumUtil->ISODateToTimeT(pKv->GetString("creationDate"), &m_tCreationDate);
    else
    {
        m_tCreationDate = (time_t)pKv->GetUint64("creationDate");
        m_bValid = true;
    }
}

void MapInfo::ToKV(KeyValues* pKv) const
{
    pKv->SetString("description", m_szDescription);
    pKv->SetInt("numBonuses", m_iNumBonuses);
    pKv->SetInt("numZones", m_iNumZones);
    pKv->SetInt("difficulty", m_iDifficulty);
    pKv->SetBool("isLinear", m_bIsLinear);
    pKv->SetUint64("creationDate", m_tCreationDate);
}

MapInfo& MapInfo::operator=(const MapInfo& other)
{
    Q_strncpy(m_szDescription, other.m_szDescription, sizeof(m_szDescription));
    m_iNumBonuses = other.m_iNumBonuses;
    m_iNumZones = other.m_iNumZones;
    m_bIsLinear = other.m_bIsLinear;
    m_iDifficulty = other.m_iDifficulty;
    m_tCreationDate = other.m_tCreationDate;
    m_bValid = other.m_bValid;
    return *this;
}

void MapImage::FromKV(KeyValues* pKv)
{
    m_uID = pKv->GetInt("id");
    Q_strncpy(m_szURLSmall, pKv->GetString("small"), sizeof(m_szURLSmall));
    Q_strncpy(m_szURLMedium, pKv->GetString("medium"), sizeof(m_szURLMedium));
    Q_strncpy(m_szURLLarge, pKv->GetString("large"), sizeof(m_szURLLarge));
    Q_strncpy(m_szLastUpdatedDate, pKv->GetString("updatedAt"), sizeof(m_szLastUpdatedDate));
    m_bValid = true;
}

void MapImage::ToKV(KeyValues* pKv) const
{
    pKv->SetInt("id", m_uID);
    pKv->SetString("small", m_szURLSmall);
    pKv->SetString("medium", m_szURLMedium);
    pKv->SetString("large", m_szURLLarge);
    pKv->SetString("updatedAt", m_szLastUpdatedDate);
}

MapImage& MapImage::operator=(const MapImage& other)
{
    m_uID = other.m_uID;
    Q_strncpy(m_szURLSmall, other.m_szURLSmall, sizeof(m_szURLSmall));
    Q_strncpy(m_szURLMedium, other.m_szURLMedium, sizeof(m_szURLMedium));
    Q_strncpy(m_szURLLarge, other.m_szURLLarge, sizeof(m_szURLLarge));
    Q_strncpy(m_szLastUpdatedDate, other.m_szLastUpdatedDate, sizeof(m_szLastUpdatedDate));
    m_bValid = other.m_bValid;
    return *this;
}

MapGallery::MapGallery(const MapGallery& other)
{
    m_Thumbnail = other.m_Thumbnail;
    m_vecExtraImages.AddVectorToTail(other.m_vecExtraImages);
    m_bValid = other.m_bValid;
}

void MapGallery::FromKV(KeyValues* pKv)
{
    KeyValues* pThumbnail = pKv->FindKey("thumbnail");
    if (pThumbnail)
    {
        m_Thumbnail.FromKV(pThumbnail);
        m_bValid = true;
    }
    KeyValues* pExtraImages = pKv->FindKey("extraImages");
    if (pExtraImages)
    {
        FOR_EACH_SUBKEY(pExtraImages, pExtraImage)
        {
            MapImage mi;
            mi.FromKV(pExtraImage);

            if (m_bFromAPI)
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

void MapGallery::ToKV(KeyValues* pKv) const
{
    if (m_Thumbnail.m_bValid)
    {
        KeyValues* pThumbnail = new KeyValues("thumbnail");
        m_Thumbnail.ToKV(pThumbnail);
        pKv->AddSubKey(pThumbnail);
    }

    if (!m_vecExtraImages.IsEmpty())
    {
        KeyValues* pExtras = new KeyValues("extraImages");
        FOR_EACH_VEC(m_vecExtraImages, i)
        {
            KeyValues* pExtraImage = pExtras->CreateNewKey();
            m_vecExtraImages[i].ToKV(pExtraImage);
        }
        pKv->AddSubKey(pExtras);
    }
}

MapGallery& MapGallery::operator=(const MapGallery& src)
{
    m_Thumbnail = src.m_Thumbnail;
    m_vecExtraImages.RemoveAll();
    m_vecExtraImages.AddVectorToTail(src.m_vecExtraImages);
    m_bValid = src.m_bValid;
    return *this;
}

void MapCredit::FromKV(KeyValues* pKv)
{
    m_uID = pKv->GetInt("id");
    m_eType = (MAP_CREDIT_TYPE)pKv->GetInt("type", -1);
    KeyValues* pUser = pKv->FindKey("user");
    if (pUser)
    {
        m_User.FromKV(pUser);
        m_bValid = true;
    }
}

void MapCredit::ToKV(KeyValues* pKv) const
{
    pKv->SetInt("id", m_uID);
    pKv->SetInt("type", m_eType);
    if (m_User.m_bValid)
    {
        KeyValues* pUser = new KeyValues("user");
        m_User.ToKV(pUser);
        pKv->AddSubKey(pUser);
    }
}

bool MapCredit::operator==(const MapCredit& other) const
{
    return m_uID == other.m_uID;
}

MapCredit& MapCredit::operator=(const MapCredit& other)
{
    m_uID = other.m_uID;
    m_eType = other.m_eType;
    m_User = other.m_User;
    m_bValid = other.m_bValid;
    return *this;
}

void Run::FromKV(KeyValues* pKv)
{
    m_uID = pKv->GetUint64("id");
    m_bIsPersonalBest = pKv->GetBool("isPersonalBest");
    m_fTickRate = pKv->GetFloat("tickRate");
    // MOM_TODO: dateAchieved: type.DATE,
    m_fTime = pKv->GetFloat("time");
    m_uFlags = pKv->GetInt("flags");
    Q_strncpy(m_szDownloadURL, pKv->GetString("file"), sizeof(m_szDownloadURL));
    Q_strncpy(m_szFileHash, pKv->GetString("hash"), sizeof(m_szFileHash));
    m_bValid = true;
}

void Run::ToKV(KeyValues* pKv) const
{
    pKv->SetUint64("id", m_uID);
    pKv->SetBool("isPersonalBest", m_bIsPersonalBest);
    pKv->SetFloat("tickRate", m_fTickRate);
    // MOM_TODO: dateAchieved: Date
    pKv->SetFloat("time", m_fTime);
    pKv->SetInt("flags", m_uFlags);
    pKv->SetString("file", m_szDownloadURL);
    pKv->SetString("hash", m_szFileHash);
}

bool Run::operator==(const Run& other) const
{
    return m_uID == other.m_uID;
}

Run& Run::operator=(const Run& other)
{
    m_uID = other.m_uID;
    m_bIsPersonalBest = other.m_bIsPersonalBest;
    m_fTickRate = other.m_fTickRate;
    // MOM_TODO: dateAchieved: type.DATE,
    m_fTime = other.m_fTime;
    m_uFlags = other.m_uFlags;
    Q_strncpy(m_szDownloadURL, other.m_szDownloadURL, sizeof(m_szDownloadURL));
    Q_strncpy(m_szFileHash, other.m_szFileHash, sizeof(m_szFileHash));
    m_bValid = other.m_bValid;
    return *this;
}

void MapRank::FromKV(KeyValues* pKv)
{
    m_iRank = pKv->GetInt("rank");
    m_iRankXP = pKv->GetInt("rankXP");

    KeyValues *pRun = pKv->FindKey("run");
    if (pRun)
    {
        m_Run.FromKV(pRun);
        m_bValid = true;
    }
}

void MapRank::ToKV(KeyValues* pKv) const
{
    pKv->SetInt("rank", m_iRank);
    pKv->SetInt("rankXP", m_iRankXP);

    if (m_Run.m_bValid)
    {
        KeyValues *pRun = new KeyValues("run");
        m_Run.ToKV(pRun);
        pKv->AddSubKey(pRun);
    }
}

bool MapRank::operator==(const MapRank& other) const
{
    return m_Run == other.m_Run && m_iRank == other.m_iRank && m_iRankXP == other.m_iRankXP;
}

MapRank& MapRank::operator=(const MapRank& other)
{
    m_iRank = other.m_iRank;
    m_iRankXP = other.m_iRankXP;
    m_Run = other.m_Run;
    m_bValid = other.m_bValid;
    return *this;
}

MapData::MapData()
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

MapData::MapData(const MapData& src)
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
    m_Thumbnail = src.m_Thumbnail;
    m_PersonalBest = src.m_PersonalBest;
    m_WorldRecord = src.m_WorldRecord;
    m_bValid = src.m_bValid;
}

void MapData::FromKV(KeyValues* pMap)
{
    m_uID = pMap->GetInt("id");
    m_eType = (GAME_MODE)pMap->GetInt("type");
    m_eMapStatus = (MAP_UPLOAD_STATUS)pMap->GetInt("statusFlag", -1);
    Q_strncpy(m_szHash, pMap->GetString("hash"), sizeof(m_szHash));
    Q_strncpy(m_szDownloadURL, pMap->GetString("downloadURL"), sizeof(m_szDownloadURL));

    Q_strncpy(m_szMapName, m_bFromAPI ? pMap->GetString("name") : pMap->GetName(), sizeof(m_szMapName));
    KeyValues *pFavorites = pMap->FindKey("favorites");
    m_bInFavorites = m_bFromAPI ? pFavorites && !pFavorites->IsEmpty() : pMap->GetBool("inFavorites");
    m_bInLibrary = m_bFromAPI ? true : pMap->GetBool("inLibrary");
    m_tLastUpdated = m_bFromAPI ? time(nullptr) : (time_t)pMap->GetUint64("lastUpdated");

    if (!m_bFromAPI)
        Q_strncpy(m_szPath, pMap->GetString("path"), sizeof(m_szPath));

    m_Info.m_bFromAPI = m_bFromAPI;

    KeyValues* pInfo = pMap->FindKey("info");
    if (pInfo)
        m_Info.FromKV(pInfo);
    KeyValues* pSubmitter = pMap->FindKey("submitter");
    if (pSubmitter)
        m_Submitter.FromKV(pSubmitter);
    KeyValues* pCredits = pMap->FindKey("credits");
    if (pCredits)
    {
        FOR_EACH_SUBKEY(pCredits, pCredit)
        {
            MapCredit mc;
            mc.FromKV(pCredit);
            if (m_bFromAPI)
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

    KeyValues *pThumbnail = pMap->FindKey("thumbnail");
    if (pThumbnail)
        m_Thumbnail.FromKV(pThumbnail);

    KeyValues *pPersonalBest = pMap->FindKey("personalBest");
    if (pPersonalBest && !pPersonalBest->IsEmpty())
        m_PersonalBest.FromKV(pPersonalBest);

    KeyValues *pWorldRecord = pMap->FindKey("worldRecord");
    if (pWorldRecord && !pWorldRecord->IsEmpty())
        m_WorldRecord.FromKV(pWorldRecord);

    m_bValid = m_Info.m_bValid && m_Submitter.m_bValid && m_Thumbnail.m_bValid;
}

void MapData::ToKV(KeyValues* pKv) const
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

    if (m_Info.m_bValid)
    {
        KeyValues* pInfo = new KeyValues("info");
        m_Info.ToKV(pInfo);
        pKv->AddSubKey(pInfo);
    }

    if (m_Submitter.m_bValid)
    {
        KeyValues* pSubmitter = new KeyValues("submitter");
        m_Submitter.ToKV(pSubmitter);
        pKv->AddSubKey(pSubmitter);
    }

    if (!m_vecCredits.IsEmpty())
    {
        KeyValues* pCredits = new KeyValues("credits");
        FOR_EACH_VEC(m_vecCredits, i)
        {
            KeyValues* pCredit = pCredits->CreateNewKey();
            m_vecCredits[i].ToKV(pCredit);
        }
        pKv->AddSubKey(pCredits);
    }

    if (m_Thumbnail.m_bValid)
    {
        KeyValues *pThumbnail = new KeyValues("thumbnail");
        m_Thumbnail.ToKV(pThumbnail);
        pKv->AddSubKey(pThumbnail);
    }

    if (m_PersonalBest.m_bValid)
    {
        KeyValues *pPersonalBest = new KeyValues("personalBest");
        m_PersonalBest.ToKV(pPersonalBest);
        pKv->AddSubKey(pPersonalBest);
    }

    if (m_WorldRecord.m_bValid)
    {
        KeyValues *pWorldRecord = new KeyValues("worldRecord");
        m_WorldRecord.ToKV(pWorldRecord);
        pKv->AddSubKey(pWorldRecord);
    }
}

MapData& MapData::operator=(const MapData& src)
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
    m_PersonalBest = src.m_PersonalBest;
    m_WorldRecord = src.m_WorldRecord;
    m_vecCredits.RemoveAll();
    m_vecCredits.AddMultipleToTail(src.m_vecCredits.Count(), src.m_vecCredits.Base());
    m_Thumbnail = src.m_Thumbnail;
    m_bValid = src.m_bValid;

    return *this;
}

bool MapData::operator==(const MapData& other) const
{
    return m_uID == other.m_uID;
}

// =============================================================================================

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
                    d.FromKV(pMap);

                    uint16 indx = m_mapMapCache.Find(d.m_uID);
                    if (m_mapMapCache.IsValidIndex(indx))
                    {
                        // Update it
                        m_mapMapCache[indx] = d;
                    }
                    else
                    {
                        // MOM_TODO check the download variable, if auto-download, do so here

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

void CMapCache::GetMapLibrary(CUtlVector<MapData*> &vecLibrary)
{
    unsigned short indx = m_mapMapCache.FirstInorder();
    while (indx != m_mapMapCache.InvalidIndex())
    {
        MapData *d = &m_mapMapCache.Element(indx);
        if (d->m_bInLibrary)
            vecLibrary.AddToTail(d);

        indx = m_mapMapCache.NextInorder(indx);
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
            d.m_bFromAPI = false;
            d.FromKV(pMap);

            m_dictMapNames.Insert(d.m_szMapName, d.m_uID);
            m_mapMapCache.Insert(d.m_uID, d);
        }
    }
    else
    {
        DevLog(2, "Map cache file doesn't exist, creating it...");
    }
}

void CMapCache::SetMapGamemode()
{
    ConVarRef gm("mom_gamemode");
    if (m_pCurrentMapData)
    {
        gm.SetValue(m_pCurrentMapData->m_eType);
    }
    else
    {
        if (!Q_strnicmp(MapName(), "surf_", 5))
        {
            gm.SetValue(GAMEMODE_SURF);
        }
        else if (!Q_strnicmp(MapName(), "bhop_", 5))
        {
            gm.SetValue(GAMEMODE_BHOP);
        }
        else if (!Q_strnicmp(MapName(), "kz_", 3))
        {
            gm.SetValue(GAMEMODE_KZ);
        }
        else
        {
            gm.SetValue(GAMEMODE_UNKNOWN);
        }
    }
}

void CMapCache::LevelInitPreEntity()
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
    else
    {
        m_pCurrentMapData = nullptr;
    }


    SetMapGamemode();
}

void CMapCache::LevelShutdownPostEntity()
{
    m_pCurrentMapData = nullptr;
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