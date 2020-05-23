#include "cbase.h"

#include "mom_api_models.h"

#include "mom_modulecomms.h"
#include "filesystem.h"
#include "fmtstr.h"

#include "tier0/memdbgon.h"

APIModel::APIModel(): m_bValid(false), m_bUpdated(true), m_eSource(MODEL_FROM_DISK)
{
}

User::User(): m_uMainID(0), m_uSteamID(0)
{
    m_szAlias[0] = '\0';
}

void User::FromKV(KeyValues* pKv)
{
    m_uMainID = pKv->GetUint64("id");
    m_uSteamID = pKv->GetUint64("steamID");
    Q_strncpy(m_szAlias, pKv->GetString("alias"), sizeof(m_szAlias));
    m_bValid = m_uMainID > 0 && Q_strlen(m_szAlias) > 0;
}

void User::ToKV(KeyValues* pKv) const
{
    pKv->SetUint64("id", m_uMainID);
    pKv->SetUint64("steamID", m_uSteamID);
    pKv->SetString("alias", m_szAlias);
}

User& User::operator=(const User& src)
{
    if (src.m_bValid)
    {
        m_bUpdated = !(*this == src);
        m_uMainID = src.m_uMainID;
        m_uSteamID = src.m_uSteamID;
        Q_strncpy(m_szAlias, src.m_szAlias, sizeof(m_szAlias));
        m_bValid = src.m_bValid;
    }

    return *this;
}

bool User::operator==(const User &other) const
{
    return m_uMainID == other.m_uMainID &&
        m_uSteamID == other.m_uSteamID && FStrEq(m_szAlias, other.m_szAlias);
}

MapInfo::MapInfo(): m_iNumTracks(0)
{
    m_szDescription[0] = '\0';
    m_szCreationDate[0] = '\0';
}

void MapInfo::FromKV(KeyValues* pKv)
{
    Q_strncpy(m_szDescription, pKv->GetString("description"), sizeof(m_szDescription));
    m_iNumTracks = pKv->GetInt("numTracks");
    Q_strncpy(m_szCreationDate, pKv->GetString("creationDate"), sizeof(m_szCreationDate));
    m_bValid = m_iNumTracks && Q_strlen(m_szDescription);
}

void MapInfo::ToKV(KeyValues* pKv) const
{
    pKv->SetString("description", m_szDescription);
    pKv->SetInt("numTracks", m_iNumTracks);
    pKv->SetString("creationDate", m_szCreationDate);
}

MapInfo& MapInfo::operator=(const MapInfo& other)
{
    if (other.m_bValid)
    {
        m_bUpdated = !(*this == other);
        Q_strncpy(m_szDescription, other.m_szDescription, sizeof(m_szDescription));
        m_iNumTracks = other.m_iNumTracks;
        Q_strncpy(m_szCreationDate, other.m_szCreationDate, sizeof(m_szCreationDate));
        m_bValid = other.m_bValid;
    }

    return *this;
}

bool MapInfo::operator==(const MapInfo &other) const
{
    return FStrEq(m_szDescription, other.m_szDescription) && m_iNumTracks == other.m_iNumTracks &&
        FStrEq(m_szCreationDate, other.m_szCreationDate);
}

MapImage::MapImage()
{
    m_uID = 0;
    m_szURLSmall[0] = '\0';
    m_szURLMedium[0] = '\0';
    m_szURLLarge[0] = '\0';
    m_szLastUpdatedDate[0] = '\0';
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

bool MapImage::operator==(const MapImage &other) const
{
    return m_uID == other.m_uID;
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

MapCredit::MapCredit(): m_uID(0), m_eType(CREDIT_UNKNOWN)
{
}

void MapCredit::FromKV(KeyValues* pKv)
{
    m_uID = pKv->GetInt("id");
    m_eType = (MapCreditType_t) pKv->GetInt("type", CREDIT_UNKNOWN);
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

Run::Run(): m_uID(0), m_bIsPersonalBest(false), m_fTickRate(0.0f), m_fTime(.0f), m_uFlags(0)
{
    m_szDownloadURL[0] = '\0';
    m_szFileHash[0] = '\0';
    m_szDateAchieved[0] = '\0';
}

void Run::FromKV(KeyValues* pKv)
{
    m_uID = pKv->GetUint64("id");
    m_bIsPersonalBest = pKv->GetBool("isPersonalBest");
    m_fTickRate = pKv->GetFloat("tickRate");
    Q_strncpy(m_szDateAchieved, pKv->GetString("createdAt"), sizeof(m_szDateAchieved));
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
    pKv->SetString("createdAt", m_szDateAchieved);
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

MapRank::MapRank(): m_iRank(0), m_iRankXP(0)
{
}

bool MapRank::NeedsUpdate() const
{
    return m_bUpdated || m_Run.m_bUpdated;
}

void MapRank::ResetUpdate()
{
    m_bUpdated = m_Run.m_bUpdated = false;
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

MapTrack::MapTrack()
{
    m_iTrackNum = 0;
    m_iDifficulty = 1;
    m_iNumZones = 0;
    m_bIsLinear = false;
}

void MapTrack::FromKV(KeyValues *pKv)
{
    m_iTrackNum = (uint8) pKv->GetInt("trackNum");
    m_iDifficulty = (uint8) pKv->GetInt("difficulty");
    m_iNumZones = (uint8) pKv->GetInt("numZones");
    m_bIsLinear = pKv->GetBool("isLinear");
    m_bValid = m_iNumZones && m_iDifficulty;
}

void MapTrack::ToKV(KeyValues *pKv) const
{
    pKv->SetInt("trackNum", m_iTrackNum);
    pKv->SetInt("difficulty", m_iDifficulty);
    pKv->SetInt("numZones", m_iNumZones);
    pKv->SetBool("isLinear", m_bIsLinear);
}

bool MapTrack::operator==(const MapTrack &other) const
{
    return m_iTrackNum == other.m_iTrackNum && m_iDifficulty == other.m_iDifficulty &&
        m_bIsLinear == other.m_bIsLinear && m_iNumZones == other.m_iNumZones;
}

MapTrack & MapTrack::operator=(const MapTrack &other)
{
    if (other.m_bValid)
    {
        m_bUpdated = !(*this == other);
        m_iTrackNum = other.m_iTrackNum;
        m_iDifficulty = other.m_iDifficulty;
        m_iNumZones = other.m_iNumZones;
        m_bIsLinear = other.m_bIsLinear;
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
    m_MainTrack = src.m_MainTrack;
    m_Submitter = src.m_Submitter;
    m_vecCredits.RemoveAll();
    m_vecCredits.AddVectorToTail(src.m_vecCredits);
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
    return m_bUpdated || m_Info.m_bUpdated || m_PersonalBest.NeedsUpdate() || m_MainTrack.m_bUpdated ||
        m_WorldRecord.NeedsUpdate() || m_Thumbnail.m_bUpdated;
}

void MapData::SendDataUpdate()
{
    KeyValues *pEvent = new KeyValues("map_data_update");

    pEvent->SetInt("id", m_uID);
    pEvent->SetBool("main", m_bUpdated);
    pEvent->SetBool("info", m_Info.m_bUpdated || m_MainTrack.m_bUpdated);
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
    m_bUpdated = m_Info.m_bUpdated = m_Thumbnail.m_bUpdated = m_MainTrack.m_bUpdated = false;
}

bool MapData::GetCreditString(CUtlString *pOut, MapCreditType_t creditType)
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

    const auto count = people.Count();

    if (count == 0)
        return false;

    if (count > 3)
    {
        pOut->Format("%s, %s and %i others", people[0], people[1], count - 2);
    }
    else if (count == 3)
    {
        pOut->Format("%s, %s and %s", people[0], people[1], people[2]);
    }
    else if (count == 2)
    {
        pOut->Format("%s and %s", people[0], people[1]);
    }
    else
    {
        pOut->Append(people[0]);
    }

    return true;
}

void MapData::DeleteMapFile()
{
    if (m_bMapFileExists)
    {
        // MOM_TODO when we allow different map library folders, construct full path here
        const char *pFilePath = CFmtStr("maps/%s.bsp", m_szMapName).Get();
        if (g_pFullFileSystem->FileExists(pFilePath, "MOD"))
            g_pFullFileSystem->RemoveFile(pFilePath, "MOD");
        m_bMapFileExists = false;
    }
}

void MapData::FromKV(KeyValues* pMap)
{
    m_uID = pMap->GetInt("id");
    m_eType = (GameMode_t) pMap->GetInt("type");
    m_eMapStatus = (MapUploadStatus_t) pMap->GetInt("statusFlag", -1);
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
        m_bInFavorites = (m_eSource == MODEL_FROM_FAVORITES_API_CALL) || (pFavorites && !pFavorites->IsEmpty());
        KeyValues *pLibrary = pMap->FindKey("libraryEntries");
        m_bInLibrary = (m_eSource == MODEL_FROM_LIBRARY_API_CALL) || (pLibrary && !pLibrary->IsEmpty());
        m_bMapFileNeedsUpdate = m_eSource == MODEL_FROM_LIBRARY_API_CALL;
    }

    KeyValues* pInfo = pMap->FindKey("info");
    if (pInfo)
        m_Info.FromKV(pInfo);
    KeyValues *pMainTrack = pMap->FindKey("mainTrack");
    if (pMainTrack)
        m_MainTrack.FromKV(pMainTrack);
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
                const auto indx = m_vecCredits.Find(mc);
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

    if (m_MainTrack.m_bValid)
    {
        KeyValues *pMainTrack = new KeyValues("mainTrack");
        m_MainTrack.ToKV(pMainTrack);
        pKv->AddSubKey(pMainTrack);
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
    m_MainTrack = src.m_MainTrack;
    m_Submitter = src.m_Submitter;
    m_PersonalBest = src.m_PersonalBest;
    m_WorldRecord = src.m_WorldRecord;
    if (src.m_vecCredits.Count())
    {
        m_vecCredits.RemoveAll();
        m_vecCredits.AddVectorToTail(src.m_vecCredits);
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