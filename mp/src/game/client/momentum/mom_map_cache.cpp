#include "cbase.h"

#include <ctime>

#include "mom_map_cache.h"
#include "mom_api_requests.h"
#include "util/mom_util.h"

#include "filesystem.h"
#include "fmtstr.h"

#include "tier0/memdbgon.h"

#define MAP_CACHE_FILE_NAME "map_cache.dat"

void User::FromKV(KeyValues* pKv)
{
    m_uID = pKv->GetUint64("id");
    Q_strncpy(m_szAlias, pKv->GetString("alias"), sizeof(m_szAlias));
    m_bValid = m_uID > 0 && Q_strlen(m_szAlias) > 0;
}

void User::ToKV(KeyValues* pKv) const
{
    pKv->SetUint64("id", m_uID);
    pKv->SetString("alias", m_szAlias);
}

User& User::operator=(const User& src)
{
    if (src.m_bValid)
    {
        m_bUpdated = !(*this == src);
        m_uID = src.m_uID;
        Q_strncpy(m_szAlias, src.m_szAlias, sizeof(m_szAlias));
        m_bValid = src.m_bValid;
    }

    return *this;
}

void MapInfo::FromKV(KeyValues* pKv)
{
    Q_strncpy(m_szDescription, pKv->GetString("description"), sizeof(m_szDescription));
    m_iNumBonuses = pKv->GetInt("numBonuses");
    m_iNumZones = pKv->GetInt("numZones");
    m_bIsLinear = pKv->GetBool("isLinear");
    m_iDifficulty = pKv->GetInt("difficulty");
    Q_strncpy(m_szCreationDate, pKv->GetString("creationDate"), sizeof(m_szCreationDate));
    m_bValid = m_iDifficulty && Q_strlen(m_szDescription);
}

void MapInfo::ToKV(KeyValues* pKv) const
{
    pKv->SetString("description", m_szDescription);
    pKv->SetInt("numBonuses", m_iNumBonuses);
    pKv->SetInt("numZones", m_iNumZones);
    pKv->SetInt("difficulty", m_iDifficulty);
    pKv->SetBool("isLinear", m_bIsLinear);
    pKv->SetString("creationDate", m_szCreationDate);
}

MapInfo& MapInfo::operator=(const MapInfo& other)
{
    if (other.m_bValid)
    {
        m_bUpdated = !(*this == other);
        Q_strncpy(m_szDescription, other.m_szDescription, sizeof(m_szDescription));
        m_iNumBonuses = other.m_iNumBonuses;
        m_iNumZones = other.m_iNumZones;
        m_bIsLinear = other.m_bIsLinear;
        m_iDifficulty = other.m_iDifficulty;
        Q_strncpy(m_szCreationDate, other.m_szCreationDate, sizeof(m_szCreationDate));
        m_bValid = other.m_bValid;
    }

    return *this;
}

bool MapInfo::operator==(const MapInfo &other) const
{
    return FStrEq(m_szDescription, other.m_szDescription) && m_iNumZones == other.m_iNumZones &&
        m_iNumBonuses == other.m_iNumBonuses && m_iDifficulty == other.m_iDifficulty && 
        FStrEq(m_szCreationDate, other.m_szCreationDate);
}

void MapImage::FromKV(KeyValues* pKv)
{
    m_uID = pKv->GetInt("id");
    Q_strncpy(m_szURLSmall, pKv->GetString("small"), sizeof(m_szURLSmall));
    Q_strncpy(m_szURLMedium, pKv->GetString("medium"), sizeof(m_szURLMedium));
    Q_strncpy(m_szURLLarge, pKv->GetString("large"), sizeof(m_szURLLarge));
    Q_strncpy(m_szLastUpdatedDate, pKv->GetString("updatedAt"), sizeof(m_szLastUpdatedDate));
    m_bValid = m_uID > 0;
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
    if (other.m_bValid)
    {
        m_bUpdated = !(*this == other);
        m_uID = other.m_uID;
        Q_strncpy(m_szURLSmall, other.m_szURLSmall, sizeof(m_szURLSmall));
        Q_strncpy(m_szURLMedium, other.m_szURLMedium, sizeof(m_szURLMedium));
        Q_strncpy(m_szURLLarge, other.m_szURLLarge, sizeof(m_szURLLarge));
        Q_strncpy(m_szLastUpdatedDate, other.m_szLastUpdatedDate, sizeof(m_szLastUpdatedDate));
        m_bValid = other.m_bValid;
    }

    return *this;
}

void MapCredit::FromKV(KeyValues* pKv)
{
    m_uID = pKv->GetInt("id");
    m_eType = (MAP_CREDIT_TYPE)pKv->GetInt("type", CREDIT_UNKNOWN);
    KeyValues* pUser = pKv->FindKey("user");
    if (pUser)
        m_User.FromKV(pUser);
    m_bValid = m_uID > 0;
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
    if (other.m_bValid)
    {
        m_bUpdated = !(*this == other);
        m_uID = other.m_uID;
        m_eType = other.m_eType;
        m_User = other.m_User;
        m_bValid = other.m_bValid;
    }
    return *this;
}

void Run::FromKV(KeyValues* pKv)
{
    m_uID = pKv->GetUint64("id");
    m_bIsPersonalBest = pKv->GetBool("isPersonalBest");
    m_fTickRate = pKv->GetFloat("tickRate");
    Q_strncpy(m_szDateAchieved, pKv->GetString("dateAchieved"), sizeof(m_szDateAchieved));
    m_fTime = pKv->GetFloat("time");
    m_uFlags = pKv->GetInt("flags");
    Q_strncpy(m_szDownloadURL, pKv->GetString("file"), sizeof(m_szDownloadURL));
    Q_strncpy(m_szFileHash, pKv->GetString("hash"), sizeof(m_szFileHash));
    m_bValid = m_uID > 0;
}

void Run::ToKV(KeyValues* pKv) const
{
    pKv->SetUint64("id", m_uID);
    pKv->SetBool("isPersonalBest", m_bIsPersonalBest);
    pKv->SetFloat("tickRate", m_fTickRate);
    pKv->SetString("dateAchieved", m_szDateAchieved);
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
    if (other.m_bValid)
    {
        m_bUpdated = !(*this == other);
        m_uID = other.m_uID;
        m_bIsPersonalBest = other.m_bIsPersonalBest;
        m_fTickRate = other.m_fTickRate;
        Q_strncpy(m_szDateAchieved, other.m_szDateAchieved, sizeof(m_szDateAchieved));
        m_fTime = other.m_fTime;
        m_uFlags = other.m_uFlags;
        Q_strncpy(m_szDownloadURL, other.m_szDownloadURL, sizeof(m_szDownloadURL));
        Q_strncpy(m_szFileHash, other.m_szFileHash, sizeof(m_szFileHash));
        m_bValid = other.m_bValid;
    }

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

        KeyValues *pUser = pKv->FindKey("user");
        if (pUser)
        {
            m_User.FromKV(pUser);
        }
    }

    m_bValid = m_iRank > 0;
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
    if (m_User.m_bValid)
    {
        KeyValues *pUser = new KeyValues("user");
        m_User.ToKV(pUser);
        pKv->AddSubKey(pUser);
    }
}

bool MapRank::operator==(const MapRank& other) const
{
    return m_Run == other.m_Run && m_iRank == other.m_iRank && m_iRankXP == other.m_iRankXP;
}

MapRank& MapRank::operator=(const MapRank& other)
{
    if (other.m_bValid)
    {
        m_bUpdated = !(*this == other);
        m_iRank = other.m_iRank;
        m_iRankXP = other.m_iRankXP;
        m_Run = other.m_Run;
        m_User = other.m_User;
        m_bValid = other.m_bValid;
    }

    return *this;
}

MapData::MapData()
{
    m_szLastUpdated[0] = '\0';
    m_bInLibrary = false;
    m_bInFavorites = false;
    m_szPath[0] = '\0';
    m_uID = 0;
    m_szMapName[0] = '\0';
    m_szHash[0] = '\0';
    m_eType = GAMEMODE_UNKNOWN;
    m_eMapStatus = STATUS_UNKNOWN;
    m_szDownloadURL[0] = '\0';
    m_bMapUpdated = false;
}

MapData::MapData(const MapData& src)
{
    m_uID = src.m_uID;
    m_eType = src.m_eType;
    m_eMapStatus = src.m_eMapStatus;
    Q_strncpy(m_szHash, src.m_szHash, sizeof(m_szHash));
    Q_strncpy(m_szDownloadURL, src.m_szDownloadURL, sizeof(m_szDownloadURL));
    m_bMapUpdated = src.m_bMapUpdated;

    Q_strncpy(m_szMapName, src.m_szMapName, sizeof(m_szMapName));
    m_bInFavorites = src.m_bInFavorites;
    m_bInLibrary = src.m_bInLibrary;
    Q_strncpy(m_szLastUpdated, src.m_szLastUpdated, sizeof(m_szLastUpdated));

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

bool MapData::WasUpdated() const
{
    return m_bUpdated || m_Info.m_bUpdated || m_PersonalBest.NeedsUpdate() || 
        m_WorldRecord.NeedsUpdate() || m_Thumbnail.m_bUpdated;
}

void MapData::SendDataUpdate()
{
    IGameEvent *pEvent = gameeventmanager->CreateEvent("map_data_update");
    if (pEvent)
    {
        pEvent->SetInt("id", m_uID);
        pEvent->SetBool("main", m_bUpdated);
        pEvent->SetBool("info", m_Info.m_bUpdated);
        pEvent->SetBool("pb", m_PersonalBest.NeedsUpdate());
        pEvent->SetBool("wr", m_WorldRecord.NeedsUpdate());
        pEvent->SetBool("thumbnail", m_Thumbnail.m_bUpdated);

        if (gameeventmanager->FireEventClientSide(pEvent))
            ResetUpdate();
    }
}

void MapData::SendMapFileUpdate()
{
    if (m_bMapUpdated)
    {
        IGameEvent *pEventMap = gameeventmanager->CreateEvent("map_file_updated");
        if (pEventMap)
        {
            pEventMap->SetInt("id", m_uID);
            gameeventmanager->FireEventClientSide(pEventMap);
            // m_bMapUpdated gets reset when the new map finishes downloading
        }
    }
}

void MapData::ResetUpdate()
{
    m_PersonalBest.ResetUpdate();
    m_WorldRecord.ResetUpdate();
    m_bUpdated = m_Info.m_bUpdated = m_Thumbnail.m_bUpdated = false;
}

bool MapData::GetCreditString(CUtlString *pOut, MAP_CREDIT_TYPE creditType)
{
    if (m_vecCredits.IsEmpty() || !pOut)
        return false;

    CUtlStringList people;
    FOR_EACH_VEC(m_vecCredits, i)
    {
        if (m_vecCredits[i].m_eType == creditType)
        {
            people.CopyAndAddToTail(m_vecCredits[i].m_User.m_szAlias);
        }
    }
    int count = people.Count();
    for (int i = 0; i < count; i++)
    {
        pOut->Append(people[i]);
        if (i < count - 2)
            pOut->Append(", ", 2);
    }
    return true;
}

void MapData::FromKV(KeyValues* pMap)
{
    m_uID = pMap->GetInt("id");
    m_eType = (GAME_MODE)pMap->GetInt("type");
    m_eMapStatus = (MAP_UPLOAD_STATUS)pMap->GetInt("statusFlag", -1);
    Q_strncpy(m_szHash, pMap->GetString("hash"), sizeof(m_szHash));
    Q_strncpy(m_szDownloadURL, pMap->GetString("downloadURL"), sizeof(m_szDownloadURL));
    Q_strncpy(m_szLastUpdated, pMap->GetString("updatedAt"), sizeof(m_szLastUpdated));

    if (m_eSource == MODEL_FROM_DISK)
    {
        Q_strncpy(m_szMapName, pMap->GetName(), sizeof(m_szMapName));
        m_bInFavorites = pMap->GetBool("inFavorites");
        m_bInLibrary = pMap->GetBool("inLibrary");
        Q_strncpy(m_szPath, pMap->GetString("path"), sizeof(m_szPath));
        m_bMapUpdated = pMap->GetBool("mapNeedsUpdate");
    }
    else
    {
        Q_strncpy(m_szMapName, pMap->GetString("name"), sizeof(m_szMapName));
        KeyValues *pFavorites = pMap->FindKey("favorites");
        m_bInFavorites = m_eSource == MODEL_FROM_FAVORITES_API_CALL || pFavorites && !pFavorites->IsEmpty();
        KeyValues *pLibrary = pMap->FindKey("libraryEntries");
        m_bInLibrary = m_eSource == MODEL_FROM_LIBRARY_API_CALL || pLibrary && !pLibrary->IsEmpty();
    }

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
            if (m_eSource > MODEL_FROM_DISK)
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

    m_bValid = m_uID > 0;
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
    pKv->SetString("updatedAt", m_szLastUpdated);
    pKv->SetBool("mapNeedsUpdate", m_bMapUpdated);
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
    if (src.m_bValid)
    {
        m_bUpdated = !(*this == src);
        
        m_uID = src.m_uID;
        m_eType = src.m_eType;
        m_eMapStatus = src.m_eMapStatus;

        m_bMapUpdated = !m_bValid || !FStrEq(m_szHash, src.m_szHash);

        Q_strncpy(m_szHash, src.m_szHash, sizeof(m_szHash));
        Q_strncpy(m_szDownloadURL, src.m_szDownloadURL, sizeof(m_szDownloadURL));

        Q_strncpy(m_szMapName, src.m_szMapName, sizeof(m_szMapName));
        m_bInFavorites = src.m_bInFavorites;
        m_bInLibrary = src.m_bInLibrary;
        Q_strncpy(m_szLastUpdated, src.m_szLastUpdated, sizeof(m_szLastUpdated));

        Q_strncpy(m_szPath, src.m_szPath, sizeof(m_szPath));

        m_bValid = src.m_bValid;
    }

    m_Info = src.m_Info;
    m_Submitter = src.m_Submitter;
    m_PersonalBest = src.m_PersonalBest;
    m_WorldRecord = src.m_WorldRecord;
    if (src.m_vecCredits.Count())
    {
        m_vecCredits.RemoveAll();
        m_vecCredits.AddMultipleToTail(src.m_vecCredits.Count(), src.m_vecCredits.Base());
    }
    m_Thumbnail = src.m_Thumbnail;

    return *this;
}

bool MapData::operator==(const MapData& other) const
{
    return m_uID == other.m_uID && FStrEq(m_szLastUpdated, other.m_szLastUpdated) &&
        FStrEq(m_szMapName, other.m_szMapName) &&
        FStrEq(m_szHash, other.m_szHash) &&
        m_eType == other.m_eType && 
        m_eMapStatus == other.m_eMapStatus &&
        m_bInFavorites == other.m_bInFavorites && m_bInLibrary == other.m_bInLibrary && 
        m_Info == other.m_Info && m_PersonalBest == other.m_PersonalBest && m_WorldRecord == other.m_WorldRecord &&
        m_Thumbnail == other.m_Thumbnail;
}

// =============================================================================================

CMapCache::CMapCache() : CAutoGameSystem("CMapCache"), m_pCurrentMapData(nullptr)
{
    SetDefLessFunc(m_mapMapCache);
    SetDefLessFunc(m_mapFileDownloads);
}

bool CMapCache::PlayMap(uint32 uID)
{
    MapData *dat = GetMapDataByID(uID);
    if (dat)
    {
        const char *pMapName = dat->m_szMapName;
        // Firstly, check if we have this version of the map, and exit early
        const char *pFilePath = CFmtStr("maps/%s.bsp", pMapName).Get();
        if (g_pMomentumUtil->FileExists(pFilePath, dat->m_szHash))
        {
            engine->ClientCmd_Unrestricted(CFmtStr("map %s\n", pMapName));
            return true;
        }

        // Check if we're already downloading it
        bool bFound = false;
        unsigned short indx = m_mapFileDownloads.FirstInorder();
        while (indx != m_mapFileDownloads.InvalidIndex())
        {
            if (m_mapFileDownloads[indx] == uID)
            {
                bFound = true;
                break;
            }

            indx = m_mapFileDownloads.NextInorder(indx);
        }
        if (bFound)
        {
            // Already downloading!
            Log("Already downloading map %s!\n", pMapName);
        }
        else if (uID)
        {
            // We either don't have it, or it's outdated, so let's get the latest one!
            HTTPRequestHandle handle = g_pAPIRequests->DownloadFile(dat->m_szDownloadURL,
                                                                    UtlMakeDelegate(this, &CMapCache::StartMapDownload),
                                                                    UtlMakeDelegate(this, &CMapCache::MapDownloadProgress),
                                                                    UtlMakeDelegate(this, &CMapCache::FinishMapDownload),
                                                                    pFilePath, "GAME");
            if (handle != INVALID_HTTPREQUEST_HANDLE)
            {
                m_mapFileDownloads.Insert(handle, uID);
            }
            else
            {
                Warning("Failed to try to download the map %s!\n", pMapName);
            }
        }
    }

    return false;
}

bool CMapCache::AddMapToLibrary(uint32 uID)
{
    return g_pAPIRequests->AddMapToLibrary(uID, UtlMakeDelegate(this, &CMapCache::OnMapAddedToLibrary));
}

bool CMapCache::RemoveMapFromLibrary(uint32 uID)
{
    return g_pAPIRequests->RemoveMapFromLibrary(uID, UtlMakeDelegate(this, &CMapCache::OnMapRemovedFromLibrary));
}

bool CMapCache::AddMapToFavorites(uint32 uID)
{
    return g_pAPIRequests->AddMapToFavorites(uID, UtlMakeDelegate(this, &CMapCache::OnMapAddedToFavorites));
}

bool CMapCache::RemoveMapFromFavorites(uint32 uID)
{
    return g_pAPIRequests->RemoveMapFromFavorites(uID, UtlMakeDelegate(this, &CMapCache::OnMapRemovedFromFavorites));
}

void CMapCache::FireGameEvent(IGameEvent* event)
{
    // event->GetName() == "site_auth"
    if (g_pAPIRequests->IsAuthenticated())
    {
        if (g_pAPIRequests->GetUserMapLibrary(UtlMakeDelegate(this, &CMapCache::OnFetchPlayerMapLibrary)))
            DevLog("Requested the library!\n");

        if (g_pAPIRequests->GetUserMapFavorites(UtlMakeDelegate(this, &CMapCache::OnFetchPlayerMapFavorites)))
            DevLog("Requested favorites!\n");
    }
    else
    {
        DevWarning("We're not authenticated!\n");
    }
}

MapData *CMapCache::GetMapDataByID(uint32 uMapID)
{
    const auto indx = m_mapMapCache.Find(uMapID);
    if (m_mapMapCache.IsValidIndex(indx))
    {
        return &m_mapMapCache.Element(indx);
    }

    return nullptr;
}

void CMapCache::GetMapList(CUtlVector<MapData*>& vecMaps, MapListType_e type)
{
    auto indx = m_mapMapCache.FirstInorder();
    while (indx != m_mapMapCache.InvalidIndex())
    {
        MapData *d = &m_mapMapCache.Element(indx);
        bool bShouldAdd = d->m_eMapStatus == MAP_APPROVED;
        if (type == MAP_LIST_LIBRARY)
            bShouldAdd = d->m_bInLibrary;
        else if (type == MAP_LIST_FAVORITES)
            bShouldAdd = d->m_bInFavorites;
        else if (type == MAP_LIST_TESTING)
            bShouldAdd = d->m_eMapStatus == MAP_PRIVATE_TESTING || d->m_eMapStatus == MAP_PUBLIC_TESTING;

        if (bShouldAdd)
            vecMaps.AddToTail(&m_mapMapCache[indx]);

        indx = m_mapMapCache.NextInorder(indx);
    }
}

bool CMapCache::AddMapsToCache(KeyValues* pData, APIModelSource source)
{
    if (!pData || pData->IsEmpty())
        return false;

    FOR_EACH_SUBKEY(pData, pMap)
    {
        KeyValues *pMapPtr = pMap;
        if (source == MODEL_FROM_LIBRARY_API_CALL || source == MODEL_FROM_FAVORITES_API_CALL)
            pMapPtr = pMap->FindKey("map");

        AddMapToCache(pMapPtr, source);
    }

    if (source != MODEL_FROM_DISK)
    {
        FireMapCacheUpdateEvent(source);
    }

    return true;
}

void CMapCache::AddMapToCache(KeyValues* pMap, APIModelSource source)
{
    MapData d;
    d.m_eSource = source;
    d.FromKV(pMap);

    auto indx = m_mapMapCache.Find(d.m_uID);
    if (m_mapMapCache.IsValidIndex(indx))
    {
        // Update it
        m_mapMapCache[indx] = d;
        // Update other UI about this update if need be
        if (m_mapMapCache[indx].WasUpdated())
            m_mapMapCache[indx].SendDataUpdate();
        // If the map file itself needs updating, send the event here
        if (m_mapMapCache[indx].m_bMapUpdated)
            m_mapMapCache[indx].SendMapFileUpdate();
    }
    else
    {
        m_dictMapNames.Insert(d.m_szMapName, d.m_uID);
        m_mapMapCache.Insert(d.m_uID, d);

        // Force an update event if not from disk
        if (source != MODEL_FROM_DISK)
        {
            d.SendDataUpdate();
            d.SendMapFileUpdate();
            // MOM_TODO something will need to check the download variable, if auto-download, do so here
        }
    }
}

void CMapCache::FireMapCacheUpdateEvent(APIModelSource source)
{
    IGameEvent *pEvent = gameeventmanager->CreateEvent("map_cache_updated");
    if (pEvent)
    {
        pEvent->SetInt("source", source);
        gameeventmanager->FireEventClientSide(pEvent);
    }
}

bool CMapCache::UpdateMapInfo(uint32 uMapID)
{
    MapData *pData = GetMapDataByID(uMapID);
    if (pData)
    {
        return g_pAPIRequests->GetMapInfo(uMapID, UtlMakeDelegate(this, &CMapCache::OnFetchMapInfo));
    }

    return false;
}

uint32 CMapCache::GetUpdateIntervalForMap(MapData* pData)
{
    switch (pData->m_eMapStatus)
    {
    case MAP_PRIVATE_TESTING:
    case MAP_PUBLIC_TESTING:
        return 300; // 5 minutes
    case MAP_APPROVED:
    default:
        return 3600; // One hour
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

void CMapCache::OnFetchPlayerMapLibrary(KeyValues* pKv)
{
    KeyValues *pData = pKv->FindKey("data");
    KeyValues *pErr = pKv->FindKey("error");

    if (pData)
    {
        KeyValues *pEntries = pData->FindKey("entries");
        AddMapsToCache(pEntries, MODEL_FROM_LIBRARY_API_CALL);
    }
    else if (pErr)
    {
        // MOM_TODO error handle here
    }
}

void CMapCache::OnFetchPlayerMapFavorites(KeyValues* pKv)
{
    KeyValues *pData = pKv->FindKey("data");
    KeyValues *pErr = pKv->FindKey("error");

    if (pData)
    {
        KeyValues *pEntries = pData->FindKey("favorites");
        AddMapsToCache(pEntries, MODEL_FROM_FAVORITES_API_CALL);
    }
    else if (pErr)
    {
        // MOM_TODO error handle here
    }
}

void CMapCache::OnFetchMapInfo(KeyValues* pKv)
{
    KeyValues *pData = pKv->FindKey("data");
    KeyValues *pErr = pKv->FindKey("error");

    if (pData)
    {
        AddMapToCache(pData, MODEL_FROM_INFO_API_CALL);
        FireMapCacheUpdateEvent(MODEL_FROM_INFO_API_CALL);
    }
    else if (pErr)
    {
        // MOM_TODO error handle here
    }
}

void CMapCache::OnMapAddedToLibrary(KeyValues* pKv)
{
    KeyValues *pData = pKv->FindKey("data");
    KeyValues *pErr = pKv->FindKey("error");
    if (pData)
    {
        // Actually update the map in question, fire off an update
        int id = pData->GetInt("mapID");
        MapData *pMapData = GetMapDataByID(id);
        if (pMapData)
        {
            pMapData->m_bInLibrary = true;
            pMapData->m_bUpdated = true;
            pMapData->SendDataUpdate();
        }
    }
    else if (pErr)
    {
        // MOM_TODO error handle here
    }
}

void CMapCache::OnMapRemovedFromLibrary(KeyValues* pKv)
{
    KeyValues *pData = pKv->FindKey("data");
    KeyValues *pErr = pKv->FindKey("error");
    if (pData)
    {
        // Actually update the map in question, fire off an update
        int id = pData->GetInt("mapID");
        MapData *pMapData = GetMapDataByID(id);
        if (pMapData)
        {
            pMapData->m_bInLibrary = false;
            pMapData->m_bUpdated = true;
            pMapData->SendDataUpdate();
        }
    }
    else if (pErr)
    {
        // MOM_TODO error handle here
    }
}

void CMapCache::OnMapAddedToFavorites(KeyValues* pKv)
{
    KeyValues *pData = pKv->FindKey("data");
    KeyValues *pErr = pKv->FindKey("error");
    if (pData)
    {
        // Actually update the map in question, fire off an update
        int id = pData->GetInt("mapID");
        MapData *pMapData = GetMapDataByID(id);
        if (pMapData)
        {
            pMapData->m_bInFavorites = true;
            pMapData->m_bUpdated = true;
            pMapData->SendDataUpdate();
        }
    }
    else if (pErr)
    {
        // MOM_TODO error handle here
    }
}

void CMapCache::OnMapRemovedFromFavorites(KeyValues* pKv)
{
    KeyValues *pData = pKv->FindKey("data");
    KeyValues *pErr = pKv->FindKey("error");
    if (pData)
    {
        // Actually update the map in question, fire off an update
        int id = pData->GetInt("mapID");
        MapData *pMapData = GetMapDataByID(id);
        if (pMapData)
        {
            pMapData->m_bInFavorites = false;
            pMapData->m_bUpdated = true;
            pMapData->SendDataUpdate();
        }
    }
    else if (pErr)
    {
        // MOM_TODO error handle here
    }
}

void CMapCache::StartMapDownload(KeyValues* pKvHeader)
{
    // MOM_TODO: Create the progress bar here
}

void CMapCache::MapDownloadProgress(KeyValues* pKvProgress)
{
    uint16 fileIndx = m_mapFileDownloads.Find(pKvProgress->GetUint64("request"));
    if (fileIndx != m_mapFileDownloads.InvalidIndex())
    {
        DevLog("Progress: %0.2f!\n", pKvProgress->GetFloat("percent"));

        // MOM_TODO: update the progress bar here, but do not use the percent! Use the offset and size of the chunk!
        // Percent seems to be cached, i.e. sends a lot of "100%" if Steam downloaded the file and is sending the chunks from cache to us
    }
}

void CMapCache::FinishMapDownload(KeyValues* pKvComplete)
{
    uint16 fileIndx = m_mapFileDownloads.Find(pKvComplete->GetUint64("request"));
    if (fileIndx != m_mapFileDownloads.InvalidIndex())
    {
        if (pKvComplete->GetBool("error"))
        {
            // MOM_TODO: Show some sort of error icon on the progress bar
            Warning("Could not download map! Error code: %i\n", pKvComplete->GetInt("code"));
        }
        else
        {
            // MOM_TODO: show success on the progress bar here
            DevLog("Successfully downloaded the map with ID: %i\n", m_mapFileDownloads[fileIndx]);
        }

        m_mapFileDownloads.RemoveAt(fileIndx);
    }
}

void CMapCache::PostInit()
{
    ListenForGameEvent("site_auth");

    // Load the cache from disk
    LoadMapCacheFromDisk();
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

            // Check the update need & severity
            /*if (time(nullptr) - m_pCurrentMapData->m_tLastUpdateCheck > GetUpdateIntervalForMap(m_pCurrentMapData))
            {
                // Needs update
                DevLog("The map needs updating!\n");
                UpdateMapInfo(m_pCurrentMapData->m_uID);
            }*/
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
    SaveMapCacheToDisk();
}

void CMapCache::LoadMapCacheFromDisk()
{
    KeyValuesAD pMapData("MapCacheData");
    if (pMapData->LoadFromFile(g_pFullFileSystem, MAP_CACHE_FILE_NAME, "MOD"))
    {
        AddMapsToCache(pMapData, MODEL_FROM_DISK);
    }
    else
    {
        DevLog(2, "Map cache file doesn't exist, creating it...");
    }
}

void CMapCache::SaveMapCacheToDisk()
{
    KeyValuesAD pMapData("MapCacheData");

    auto indx = m_mapMapCache.FirstInorder();
    while (indx != m_mapMapCache.InvalidIndex())
    {
        KeyValues *pMap = pMapData->CreateNewKey();
        m_mapMapCache[indx].ToKV(pMap);

        indx = m_mapMapCache.NextInorder(indx);
    }

    if (!pMapData->SaveToFile(g_pFullFileSystem, MAP_CACHE_FILE_NAME, "MOD"))
        DevLog("Failed to log map cache out to file\n");
}

CMapCache s_mapCache;
CMapCache *g_pMapCache = &s_mapCache;