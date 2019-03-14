#include "cbase.h"

#include <ctime>

#include "mom_map_cache.h"
#include "mom_api_requests.h"
#include "util/mom_util.h"
#include "mom_modulecomms.h"

#include "filesystem.h"
#include "fmtstr.h"

#include "tier0/memdbgon.h"

#define MAP_CACHE_FILE_NAME "map_cache.dat"

void DownloadQueueCallback(IConVar *var, const char *pOldValue, float flOldValue)
{
    g_pMapCache->OnDownloadQueueToggled();
}

void DownloadQueueParallelCallback(IConVar *var, const char *pOldValue, float flOldValue)
{
    g_pMapCache->OnDownloadQueueSizeChanged();   
}

static MAKE_TOGGLE_CONVAR(mom_map_delete_queue, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED, "If 1, maps will be queued to be deleted upon game close.\nIf 0, maps are deleted the moment they are confirmed to have been removed from library.\n");
static MAKE_TOGGLE_CONVAR(mom_map_download_auto, "0", FCVAR_ARCHIVE | FCVAR_REPLICATED, "If 1, maps will automatically download when updated/added to library.\n");
static MAKE_TOGGLE_CONVAR_C(mom_map_download_queue, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED, "If 1, maps will be queued to download, allowing mom_map_download_queue_parallel parallel downloads.\n", DownloadQueueCallback);
static MAKE_CONVAR_C(mom_map_download_queue_parallel, "3", FCVAR_ARCHIVE | FCVAR_REPLICATED, "The number of parallel map downloads if mom_map_download_queue is 1.\n", 1, 20, DownloadQueueParallelCallback);
static MAKE_TOGGLE_CONVAR(mom_map_download_cancel_confirm, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED, "If 1, a messagebox will be created to ask to confirm cancelling downloads.\n");

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
    m_bValid = m_iNumZones && Q_strlen(m_szDescription);
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
    m_szCreatedAt[0] = '\0';
    m_bInLibrary = false;
    m_bInFavorites = false;
    m_bMapFileExists = false;
    m_uID = 0;
    m_szMapName[0] = '\0';
    m_szHash[0] = '\0';
    m_eType = GAMEMODE_UNKNOWN;
    m_eMapStatus = STATUS_UNKNOWN;
    m_szDownloadURL[0] = '\0';
    m_bMapFileNeedsUpdate = false;
    m_tLastPlayed = 0;
}

MapData::MapData(const MapData& src)
{
    m_uID = src.m_uID;
    m_eType = src.m_eType;
    m_eMapStatus = src.m_eMapStatus;
    Q_strncpy(m_szHash, src.m_szHash, sizeof(m_szHash));
    Q_strncpy(m_szDownloadURL, src.m_szDownloadURL, sizeof(m_szDownloadURL));
    m_bMapFileNeedsUpdate = src.m_bMapFileNeedsUpdate;
    m_bMapFileExists = src.m_bMapFileExists;

    Q_strncpy(m_szMapName, src.m_szMapName, sizeof(m_szMapName));
    m_bInFavorites = src.m_bInFavorites;
    m_bInLibrary = src.m_bInLibrary;
    Q_strncpy(m_szLastUpdated, src.m_szLastUpdated, sizeof(m_szLastUpdated));
    Q_strncpy(m_szCreatedAt, src.m_szCreatedAt, sizeof(m_szCreatedAt));

    m_Info = src.m_Info;
    m_Submitter = src.m_Submitter;
    m_vecCredits.RemoveAll();
    m_vecCredits.AddMultipleToTail(src.m_vecCredits.Count(), src.m_vecCredits.Base());
    m_vecImages.RemoveAll();
    m_vecImages.AddVectorToTail(src.m_vecImages);
    m_Thumbnail = src.m_Thumbnail;
    m_PersonalBest = src.m_PersonalBest;
    m_WorldRecord = src.m_WorldRecord;
    m_tLastPlayed = src.m_tLastPlayed;
    m_bValid = src.m_bValid;
}

bool MapData::WasUpdated() const
{
    return m_bUpdated || m_Info.m_bUpdated || m_PersonalBest.NeedsUpdate() || 
        m_WorldRecord.NeedsUpdate() || m_Thumbnail.m_bUpdated;
}

void MapData::SendDataUpdate()
{
    KeyValues *pEvent = new KeyValues("map_data_update");

    pEvent->SetInt("id", m_uID);
    pEvent->SetBool("main", m_bUpdated);
    pEvent->SetBool("info", m_Info.m_bUpdated);
    pEvent->SetBool("pb", m_PersonalBest.NeedsUpdate());
    pEvent->SetBool("wr", m_WorldRecord.NeedsUpdate());
    pEvent->SetBool("thumbnail", m_Thumbnail.m_bUpdated);

    g_pModuleComms->FireEvent(pEvent, FIRE_LOCAL_ONLY);
    ResetUpdate();
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

void MapData::DeleteMapFile()
{
    if (m_bMapFileExists)
    {
        // MOM_TODO when we allow different map library folders, construct full path here
        const char *pFilePath = CFmtStr("maps/%s.bsp", m_szMapName).Get();
        if (g_pFullFileSystem->FileExists(pFilePath, "GAME"))
            g_pFullFileSystem->RemoveFile(pFilePath, "GAME");
        m_bMapFileExists = false;
    }
}

void MapData::FromKV(KeyValues* pMap)
{
    m_uID = pMap->GetInt("id");
    m_eType = (GAME_MODE)pMap->GetInt("type");
    m_eMapStatus = (MAP_UPLOAD_STATUS)pMap->GetInt("statusFlag", -1);
    Q_strncpy(m_szHash, pMap->GetString("hash"), sizeof(m_szHash));
    Q_strncpy(m_szDownloadURL, pMap->GetString("downloadURL"), sizeof(m_szDownloadURL));
    Q_strncpy(m_szLastUpdated, pMap->GetString("updatedAt"), sizeof(m_szLastUpdated));
    Q_strncpy(m_szCreatedAt, pMap->GetString("createdAt"), sizeof(m_szCreatedAt));

    if (m_eSource == MODEL_FROM_DISK)
    {
        Q_strncpy(m_szMapName, pMap->GetName(), sizeof(m_szMapName));
        m_bInFavorites = pMap->GetBool("inFavorites");
        m_bInLibrary = pMap->GetBool("inLibrary");
        m_bMapFileExists = pMap->GetBool("mapFileExists");
        m_bMapFileNeedsUpdate = pMap->GetBool("mapNeedsUpdate");
        m_tLastPlayed = pMap->GetUint64("lastPlayed");
    }
    else
    {
        Q_strncpy(m_szMapName, pMap->GetString("name"), sizeof(m_szMapName));
        KeyValues *pFavorites = pMap->FindKey("favorites");
        m_bInFavorites = m_eSource == MODEL_FROM_FAVORITES_API_CALL || pFavorites && !pFavorites->IsEmpty();
        KeyValues *pLibrary = pMap->FindKey("libraryEntries");
        m_bInLibrary = m_eSource == MODEL_FROM_LIBRARY_API_CALL || pLibrary && !pLibrary->IsEmpty();
        m_bMapFileNeedsUpdate = m_eSource == MODEL_FROM_LIBRARY_API_CALL;
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

    KeyValues *pImages = pMap->FindKey("images");
    if (pImages)
    {
        FOR_EACH_SUBKEY(pImages, pImage)
        {
            MapImage mi;
            mi.FromKV(pImage);
            if (m_eSource > MODEL_FROM_DISK)
            {
                const auto indx = m_vecImages.Find(mi);
                if (m_vecImages.IsValidIndex(indx))
                {
                    m_vecImages[indx] = mi;
                    continue;
                }
            }

            m_vecImages.AddToTail(mi);
        }
    }

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
    pKv->SetString("createdAt", m_szCreatedAt);
    pKv->SetBool("mapNeedsUpdate", m_bMapFileNeedsUpdate);
    pKv->SetBool("mapFileExists", m_bMapFileExists);
    pKv->SetUint64("lastPlayed", m_tLastPlayed);

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
            m_vecCredits[i].ToKV(pCredits->CreateNewKey());
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

    if (!m_vecImages.IsEmpty())
    {
        KeyValues *pImages = new KeyValues("images");
        FOR_EACH_VEC(m_vecImages, i)
        {
            m_vecImages[i].ToKV(pImages->CreateNewKey());
        }
        pKv->AddSubKey(pImages);
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

        // MOM_TODO: is src.m_bInLibrary always set properly? Make sure the API requests have it!
        m_bMapFileNeedsUpdate = m_bMapFileNeedsUpdate || (src.m_bInLibrary && !FStrEq(m_szHash, src.m_szHash));

        Q_strncpy(m_szHash, src.m_szHash, sizeof(m_szHash));
        Q_strncpy(m_szDownloadURL, src.m_szDownloadURL, sizeof(m_szDownloadURL));

        Q_strncpy(m_szMapName, src.m_szMapName, sizeof(m_szMapName));
        m_bInFavorites = src.m_bInFavorites;
        m_bInLibrary = src.m_bInLibrary;
        Q_strncpy(m_szLastUpdated, src.m_szLastUpdated, sizeof(m_szLastUpdated));
        Q_strncpy(m_szCreatedAt, src.m_szCreatedAt, sizeof(m_szCreatedAt));

        if (src.m_tLastPlayed > m_tLastPlayed)
            m_tLastPlayed = src.m_tLastPlayed;

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
    if (src.m_vecImages.Count())
    {
        m_vecImages.RemoveAll();
        m_vecImages.AddVectorToTail(src.m_vecImages);
    }
    m_Thumbnail = src.m_Thumbnail;

    return *this;
}

bool MapData::operator==(const MapData& other) const
{
    return m_uID == other.m_uID && FStrEq(m_szLastUpdated, other.m_szLastUpdated) &&
        FStrEq(m_szCreatedAt, other.m_szCreatedAt) &&
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
    SetDefLessFunc(m_mapQueuedDelete);
    SetDefLessFunc(m_mapQueuedDownload);
}

CMapCache::~CMapCache()
{
    m_mapMapCache.PurgeAndDeleteElements();
}

bool CMapCache::PlayMap(uint32 uID)
{
    MapData *pData = GetMapDataByID(uID);
    if (MapFileExists(pData))
    {
        engine->ClientCmd_Unrestricted(CFmtStr("map %s\n", pData->m_szMapName));
        return true;
    }

    pData->m_bMapFileExists = false;
    pData->m_bMapFileNeedsUpdate = pData->m_bInLibrary;
    pData->m_bUpdated = true;
    pData->SendDataUpdate();

    return false;
}

bool CMapCache::MapFileExists(MapData* pData)
{
    if (!pData)
        return false;

    const char *pFilePath = CFmtStr("maps/%s.bsp", pData->m_szMapName).Get();
    return g_pMomentumUtil->FileExists(pFilePath, pData->m_szHash);
}

bool CMapCache::DownloadMap(uint32 uID)
{
    MapData *pData = GetMapDataByID(uID);
    if (pData)
    {
        // Firstly, check if we have this version of the map, and exit early
        if (MapFileExists(pData))
        {
            pData->m_bMapFileNeedsUpdate = false;
            pData->m_bMapFileExists = true;
            pData->m_bUpdated = true;
            pData->SendDataUpdate();
            Log("Map already exists!\n");
            return false;
        }

        // Check if we're already downloading it
        if (IsMapDownloading(uID))
        {
            // Already downloading!
            Log("Already downloading map %s!\n", pData->m_szMapName);
        }
        else if (IsMapQueuedToDownload(uID))
        {
            // Move it to download if our queue can fit it
            if ((unsigned)mom_map_download_queue_parallel.GetInt() > m_mapFileDownloads.Count())
            {
                return StartDownloadingMap(pData);
            }
        }
        else if (uID)
        {
            if (mom_map_download_queue.GetBool())
            {
                return AddMapToDownloadQueue(pData);
            }
            else
            {
                return StartDownloadingMap(pData);
            }
        }
    }

    return false;
}

bool CMapCache::StartDownloadingMap(MapData* pData)
{
    // We either don't have it, or it's outdated, so let's get the latest one!
    const char *pFilePath = CFmtStr("maps/%s.bsp", pData->m_szMapName).Get();
    HTTPRequestHandle handle = g_pAPIRequests->DownloadFile(pData->m_szDownloadURL,
                                                            UtlMakeDelegate(this, &CMapCache::MapDownloadSize),
                                                            UtlMakeDelegate(this, &CMapCache::MapDownloadProgress),
                                                            UtlMakeDelegate(this, &CMapCache::MapDownloadEnd),
                                                            pFilePath, "GAME", true);
    if (handle != INVALID_HTTPREQUEST_HANDLE)
    {
        m_mapFileDownloads.Insert(handle, pData->m_uID);
        // Remove from queue if it was there
        RemoveMapFromDownloadQueue(pData->m_uID);
        MapDownloadStart(pData);
        return true;
    }

    Warning("Failed to try to download the map %s!\n", pData->m_szMapName);
    return false;
}

bool CMapCache::AddMapToDownloadQueue(MapData *pData)
{
    if (!pData)
        return false;

    // Is it already queued for download?
    if (m_mapQueuedDownload.IsValidIndex(m_mapQueuedDownload.Find(pData->m_uID)))
        return false;

    // Is our current download count less than the max parallel?
    if ((unsigned)mom_map_download_queue_parallel.GetInt() > m_mapFileDownloads.Count())
        return StartDownloadingMap(pData); // Just add it to the active downloads

    m_mapQueuedDownload.Insert(pData->m_uID, pData);
    MapDownloadQueued(pData, true);

    return true;
}


bool CMapCache::CancelDownload(uint32 uID)
{
    auto indx = m_mapFileDownloads.FirstInorder();
    while (indx != m_mapFileDownloads.InvalidIndex())
    {
        if (m_mapFileDownloads[indx] == uID)
        {
            return g_pAPIRequests->CancelDownload(m_mapFileDownloads.Key(indx));
        }

        indx = m_mapFileDownloads.NextInorder(indx);
    }
    return false;
}

bool CMapCache::AddMapToDeleteQueue(MapData* pData)
{
    if (!pData || m_mapQueuedDelete.IsValidIndex(m_mapQueuedDelete.Find(pData->m_uID)))
        return false;

    m_mapQueuedDelete.Insert(pData->m_uID, pData);
    return true;
}

bool CMapCache::RemoveMapFromDeleteQueue(MapData* pData)
{
    if (!pData)
        return false;

    m_mapQueuedDelete.RemoveAt(m_mapQueuedDelete.Find(pData->m_uID));
    return true;
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
        return m_mapMapCache.Element(indx);
    }

    return nullptr;
}

void CMapCache::GetMapList(CUtlVector<MapData*>& vecMaps, MapListType_e type)
{
    auto indx = m_mapMapCache.FirstInorder();
    while (indx != m_mapMapCache.InvalidIndex())
    {
        MapData *pData = m_mapMapCache[indx];
        bool bShouldAdd = pData->m_eMapStatus == MAP_APPROVED;
        if (type == MAP_LIST_LIBRARY)
            bShouldAdd = pData->m_bInLibrary;
        else if (type == MAP_LIST_FAVORITES)
            bShouldAdd = pData->m_bInFavorites;
        else if (type == MAP_LIST_TESTING)
            bShouldAdd = pData->m_eMapStatus == MAP_PRIVATE_TESTING || pData->m_eMapStatus == MAP_PUBLIC_TESTING;

        if (bShouldAdd)
            vecMaps.AddToTail(m_mapMapCache[indx]);

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
    MapData *pData = new MapData;
    pData->m_eSource = source;
    pData->FromKV(pMap);

    const auto indx = m_mapMapCache.Find(pData->m_uID);
    if (m_mapMapCache.IsValidIndex(indx))
    {
        // Update it
        *m_mapMapCache[indx] = *pData;
        // Update other UI about this update if need be
        if (m_mapMapCache[indx]->WasUpdated())
            m_mapMapCache[indx]->SendDataUpdate();

        delete pData;
    }
    else
    {
        m_dictMapNames.Insert(pData->m_szMapName, pData->m_uID);
        m_mapMapCache.Insert(pData->m_uID, pData);

        // Force an update event if not from disk
        if (source != MODEL_FROM_DISK)
        {
            pData->SendDataUpdate();
        }
    }
}

void CMapCache::FireMapCacheUpdateEvent(APIModelSource source)
{
    KeyValues *pEvent = new KeyValues("map_cache_updated");
    pEvent->SetInt("source", source);
    g_pModuleComms->FireEvent(pEvent, FIRE_LOCAL_ONLY);
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

void CMapCache::SetMapGamemode(const char *pMapName /* = nullptr*/)
{
    ConVarRef gm("mom_gamemode");

    if (!pMapName)
        pMapName = MapName();

    if (!pMapName)
    {
        gm.SetValue(GAMEMODE_UNKNOWN);
        return;
    }

    if (m_pCurrentMapData)
    {
        gm.SetValue(m_pCurrentMapData->m_eType);
    }
    else // Backup method, use map name for unofficial maps
    {
        if (!Q_strnicmp(pMapName, "surf_", 5))
        {
            gm.SetValue(GAMEMODE_SURF);
        }
        else if (!Q_strnicmp(pMapName, "bhop_", 5))
        {
            gm.SetValue(GAMEMODE_BHOP);
        }
        else if (!Q_strnicmp(pMapName, "kz_", 3))
        {
            gm.SetValue(GAMEMODE_KZ);
        }
        else
        {
            gm.SetValue(GAMEMODE_UNKNOWN);
        }
    }
}

void CMapCache::UpdateFetchedMaps(KeyValues *pKv, bool bIsLibrary)
{
    KeyValues *pData = pKv->FindKey("data");
    KeyValues *pErr = pKv->FindKey("error");

    if (pData)
    {
        CUtlVector<MapData *> vecOldMaps;
        GetMapList(vecOldMaps, bIsLibrary ? MAP_LIST_LIBRARY : MAP_LIST_FAVORITES);

        // Precheck by setting to false
        FOR_EACH_VEC(vecOldMaps, i)
        {
            (bIsLibrary ? vecOldMaps[i]->m_bInLibrary : vecOldMaps[i]->m_bInFavorites) = false;
        }

        KeyValues *pEntries = pData->FindKey(bIsLibrary ? "entries" : "favorites");
        AddMapsToCache(pEntries, bIsLibrary ? MODEL_FROM_LIBRARY_API_CALL : MODEL_FROM_FAVORITES_API_CALL);

        // Remove ones no longer in library
        FOR_EACH_VEC(vecOldMaps, i)
        {
            if (!(bIsLibrary ? vecOldMaps[i]->m_bInLibrary : vecOldMaps[i]->m_bInFavorites))
            {
                // Actually remove it
                vecOldMaps[i]->m_bUpdated = true;
                vecOldMaps[i]->SendDataUpdate();
            }
        }
    }
    else if (pErr)
    {
        // MOM_TODO error handle here
    }
}

void CMapCache::OnFetchPlayerMapLibrary(KeyValues* pKv)
{
    UpdateFetchedMaps(pKv, true);
}

void CMapCache::OnFetchPlayerMapFavorites(KeyValues* pKv)
{
    UpdateFetchedMaps(pKv, false);
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

void CMapCache::ToggleMapLibraryOrFavorite(KeyValues* pKv, bool bIsLibrary, bool bAdded)
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
            (bIsLibrary ? pMapData->m_bInLibrary : pMapData->m_bInFavorites) = bAdded;
            pMapData->m_bUpdated = true;
            pMapData->SendDataUpdate();
        }
    }
    else if (pErr)
    {
        // MOM_TODO error handle here
    }
}

void CMapCache::OnMapAddedToLibrary(KeyValues* pKv)
{
    ToggleMapLibraryOrFavorite(pKv, true, true);
}

void CMapCache::OnMapRemovedFromLibrary(KeyValues* pKv)
{
    ToggleMapLibraryOrFavorite(pKv, true ,false);
}

void CMapCache::OnMapAddedToFavorites(KeyValues* pKv)
{
    ToggleMapLibraryOrFavorite(pKv, false, true);
}

void CMapCache::OnMapRemovedFromFavorites(KeyValues* pKv)
{
    ToggleMapLibraryOrFavorite(pKv, false, false);
}

void CMapCache::MapDownloadQueued(MapData* pData, bool bAdded)
{
    KeyValues *pKvEvent = new KeyValues("map_download_queued");
    pKvEvent->SetInt("id", pData->m_uID);
    pKvEvent->SetBool("added", bAdded);
    g_pModuleComms->FireEvent(pKvEvent, FIRE_LOCAL_ONLY);
}

void CMapCache::MapDownloadStart(MapData *pData)
{
    KeyValues *pKvEvent = new KeyValues("map_download_start");
    pKvEvent->SetInt("id", pData->m_uID);
    pKvEvent->SetString("name", pData->m_szMapName);
    g_pModuleComms->FireEvent(pKvEvent, FIRE_LOCAL_ONLY);
}

void CMapCache::MapDownloadSize(KeyValues* pKvHeader)
{
    auto indx = m_mapFileDownloads.Find(pKvHeader->GetUint64("request"));
    if (m_mapFileDownloads.IsValidIndex(indx))
    {
        uint32 id = m_mapFileDownloads[indx];

        KeyValues *pEvent = pKvHeader->MakeCopy();
        pEvent->SetName("map_download_size");
        pEvent->SetInt("id", id);
        g_pModuleComms->FireEvent(pEvent, FIRE_LOCAL_ONLY);
    }
}

void CMapCache::MapDownloadProgress(KeyValues* pKvProgress)
{
    auto fileIndx = m_mapFileDownloads.Find(pKvProgress->GetUint64("request"));
    if (fileIndx != m_mapFileDownloads.InvalidIndex())
    {
        uint32 id = m_mapFileDownloads[fileIndx];
        KeyValues *pEvent = pKvProgress->MakeCopy();
        pEvent->SetName("map_download_progress");
        pEvent->SetInt("id", id);
        g_pModuleComms->FireEvent(pEvent, FIRE_LOCAL_ONLY);
    }
}

void CMapCache::MapDownloadEnd(KeyValues* pKvComplete)
{
    uint16 fileIndx = m_mapFileDownloads.Find(pKvComplete->GetUint64("request"));
    if (fileIndx != m_mapFileDownloads.InvalidIndex())
    {
        const auto id = m_mapFileDownloads[fileIndx];

        if (pKvComplete->GetBool("error"))
        {
            int code = pKvComplete->GetInt("code");
            if (pKvComplete->GetBool("bIO") && code == k_EHTTPStatusCode410Gone) // This is internal for cancelled
                Log("Download of map %u cancelled successfully.\n", id);
            else
                Warning("Could not download map! Error code: %i\n", code);
        }
        else
        {
            MapData *pMap = GetMapDataByID(id);
            if (pMap)
            {
                pMap->m_bMapFileNeedsUpdate = false;
                pMap->m_bMapFileExists = true;
            }

            DevLog("Successfully downloaded the map with ID: %i\n", id);
        }

        m_mapFileDownloads.RemoveAt(fileIndx);

        KeyValues *pEvent = pKvComplete->MakeCopy();
        pEvent->SetName("map_download_end");
        pEvent->SetInt("id", id);
        g_pModuleComms->FireEvent(pEvent, FIRE_LOCAL_ONLY);

        // Check and add to download
        if (m_mapQueuedDownload.Count())
            DownloadMap(m_mapQueuedDownload.Key(m_mapQueuedDownload.FirstInorder()));
    }
}

bool CMapCache::IsMapDownloading(uint32 uMapID)
{
    auto indx = m_mapFileDownloads.FirstInorder();
    while (indx != m_mapFileDownloads.InvalidIndex())
    {
        if (m_mapFileDownloads[indx] == uMapID)
        {
            return true;
        }

        indx = m_mapFileDownloads.NextInorder(indx);
    }
    return false;
}

bool CMapCache::IsMapQueuedToDownload(uint32 uMapID) const
{
    return m_mapQueuedDownload.IsValidIndex(m_mapQueuedDownload.Find(uMapID));
}

void CMapCache::OnDownloadQueueSizeChanged()
{
    // We only care if it got bigger, and if we have queued maps... smaller/no queue doesn't affect anything
    if (m_mapQueuedDownload.Count())
    {
        const int diff = mom_map_download_queue_parallel.GetInt() - m_mapFileDownloads.Count();
        if (diff > 0)
        {
            int count = 0;
            // Work backwards as adding to active downloads is going to remove from here
            for(auto i = m_mapQueuedDownload.MaxElement(); m_mapQueuedDownload.IsValidIndex(i) && count < diff; i--)
            {
                if (StartDownloadingMap(m_mapQueuedDownload[i]))
                    count++;
            }
        }
    }
}

void CMapCache::OnDownloadQueueToggled()
{
    // We only care if there's a queue and it was turned off
    if (m_mapQueuedDownload.Count() && !mom_map_download_queue.GetBool())
    {
        // Only start downloading if auto is 1
        if (mom_map_download_auto.GetBool())
        {
            // Work backwards as adding to active downloads is going to remove from here
            for (auto i = m_mapQueuedDownload.MaxElement(); m_mapQueuedDownload.IsValidIndex(i); i--)
                StartDownloadingMap(m_mapQueuedDownload[i]);
        }
        else
        {
            // Cancel all outstanding queued maps
            FOR_EACH_MAP_FAST(m_mapQueuedDownload, i)
                MapDownloadQueued(m_mapQueuedDownload[i], false);
            m_mapQueuedDownload.RemoveAll();
        }
    }
}

void CMapCache::RemoveMapFromDownloadQueue(uint32 uMapID, bool bSendEvent /* = false*/)
{
    const auto indx = m_mapQueuedDownload.Find(uMapID);
    if (m_mapQueuedDownload.IsValidIndex(indx))
    {
        if (bSendEvent)
            MapDownloadQueued(m_mapQueuedDownload[indx], false);

        m_mapQueuedDownload.RemoveAt(indx);
    }
}

void CMapCache::PostInit()
{
    ListenForGameEvent("site_auth");

    g_pModuleComms->ListenForEvent("pre_level_init", UtlMakeDelegate(this, &CMapCache::PreLevelInit));

    // Load the cache from disk
    LoadMapCacheFromDisk();
}

void CMapCache::PreLevelInit(KeyValues* pKv)
{
    const char *pMapName = pKv->GetString("map");

    int dictIndx = m_dictMapNames.Find(pMapName);
    if (m_dictMapNames.IsValidIndex(dictIndx))
    {
        uint32 mapID = m_dictMapNames[dictIndx];
        uint16 mapIndx = m_mapMapCache.Find(mapID);
        if (m_mapMapCache.IsValidIndex(mapIndx))
        {
            m_pCurrentMapData = m_mapMapCache[mapIndx];

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

    SetMapGamemode(pMapName);
}

void CMapCache::LevelShutdownPostEntity()
{
    if (m_pCurrentMapData)
    {
        // Update last played
        m_pCurrentMapData->m_tLastPlayed = time(nullptr);
        m_pCurrentMapData->m_bUpdated = true;
        m_pCurrentMapData->SendDataUpdate();
    }

    m_pCurrentMapData = nullptr;
    // MOM_TODO redundant save to disk?
}

void CMapCache::Shutdown()
{
    if (m_mapQueuedDelete.Count())
    {
        FOR_EACH_MAP_FAST(m_mapQueuedDelete, i)
        {
            m_mapQueuedDelete[i]->DeleteMapFile();
        }
    }

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

    FOR_EACH_MAP(m_mapMapCache, i)
    {
        KeyValues *pMap = pMapData->CreateNewKey();
        m_mapMapCache[i]->ToKV(pMap);
    }

    if (!pMapData->SaveToFile(g_pFullFileSystem, MAP_CACHE_FILE_NAME, "MOD"))
        DevLog("Failed to log map cache out to file\n");
}

CMapCache s_mapCache;
CMapCache *g_pMapCache = &s_mapCache;