#include "cbase.h"

#include <ctime>

#include "mom_map_cache.h"
#include "mom_api_requests.h"
#include "util/mom_util.h"
#include "mom_modulecomms.h"

#include "filesystem.h"
#include "fmtstr.h"
#include "mom_system_gamemode.h"

#include "tier0/valve_minmax_off.h"
// These two are wrapped by minmax_off due to Valve making a macro for min and max...
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
// Now we can unwrap
#include "tier0/valve_minmax_on.h"

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

MAKE_TOGGLE_CONVAR(mom_map_delete_queue, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED, "If 1, maps will be queued to be deleted upon game close.\nIf 0, maps are deleted the moment they are confirmed to have been removed from library.\n");
MAKE_TOGGLE_CONVAR(mom_map_download_auto, "0", FCVAR_ARCHIVE | FCVAR_REPLICATED, "If 1, maps will automatically download when updated/added to library.\n");
MAKE_TOGGLE_CONVAR_C(mom_map_download_queue, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED, "If 1, maps will be queued to download, allowing mom_map_download_queue_parallel parallel downloads.\n", DownloadQueueCallback);
MAKE_CONVAR_C(mom_map_download_queue_parallel, "3", FCVAR_ARCHIVE | FCVAR_REPLICATED, "The number of parallel map downloads if mom_map_download_queue is 1.\n", 1, 3, DownloadQueueParallelCallback);
MAKE_TOGGLE_CONVAR(mom_map_download_cancel_confirm, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED, "If 1, a messagebox will be created to ask to confirm cancelling downloads.\n");

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
    const auto pData = GetMapDataByID(uID);
    if (pData)
    {
        if (MapFileExists(pData))
        {
            engine->ClientCmd_Unrestricted(CFmtStr("map %s\n", pData->m_szMapName));
            return true;
        }

        pData->m_bMapFileExists = false;
        pData->m_bMapFileNeedsUpdate = pData->m_bInLibrary;
        pData->m_bUpdated = true;
        pData->SendDataUpdate();
    }

    return false;
}

bool CMapCache::MapFileExists(MapData* pData)
{
    if (!pData)
        return false;

    const char *pFilePath = CFmtStr("maps/%s.bsp", pData->m_szMapName).Get();
    return MomUtil::FileExists(pFilePath, pData->m_szHash);
}

MapDownloadResponse CMapCache::DownloadMap(uint32 uID, bool bOverwrite /* = false*/)
{
    const auto pData = GetMapDataByID(uID);
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
            return MAP_DL_ALREADY_EXISTS;
        }

        // Do we have a file of the same name? If so, warn the user if they haven't told us to overwrite already
        if (!bOverwrite && g_pFullFileSystem->FileExists(CFmtStr("maps/%s.bsp", pData->m_szMapName), "MOD"))
        {
            return MAP_DL_WILL_OVERWRITE_EXISTING;
        }

        // Check if we're already downloading it
        if (IsMapDownloading(uID))
        {
            Log("Already downloading map %s!\n", pData->m_szMapName);
            return MAP_DL_ALREADY_DOWNLOADING;
        }
        
        if (IsMapQueuedToDownload(uID))
        {
            // Move it to download if our queue can fit it
            if ((unsigned)mom_map_download_queue_parallel.GetInt() > m_mapFileDownloads.Count())
            {
                if (StartDownloadingMap(pData))
                    return MAP_DL_OK;
            }
        }
        else if (uID)
        {
            if (mom_map_download_queue.GetBool())
            {
                if (AddMapToDownloadQueue(pData))
                    return MAP_DL_OK;
            }
            else
            {
                if (StartDownloadingMap(pData))
                    return MAP_DL_OK;
            }
        }
    }

    return MAP_DL_FAIL;
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
    return g_pAPIRequests->SetMapInLibrary(uID, true, UtlMakeDelegate(this, &CMapCache::OnMapAddedToLibrary));
}

bool CMapCache::RemoveMapFromLibrary(uint32 uID)
{
    return g_pAPIRequests->SetMapInLibrary(uID, false, UtlMakeDelegate(this, &CMapCache::OnMapRemovedFromLibrary));
}

bool CMapCache::AddMapToFavorites(uint32 uID)
{
    return g_pAPIRequests->SetMapInFavorites(uID, true, UtlMakeDelegate(this, &CMapCache::OnMapAddedToFavorites));
}

bool CMapCache::RemoveMapFromFavorites(uint32 uID)
{
    return g_pAPIRequests->SetMapInFavorites(uID, false, UtlMakeDelegate(this, &CMapCache::OnMapRemovedFromFavorites));
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
        else
        {
            pData->ResetUpdate();
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
    if (!pMapName)
        pMapName = MapName();

    if (m_pCurrentMapData)
    {
        g_pGameModeSystem->SetGameMode(m_pCurrentMapData->m_eType);
    }
    else // Backup method, use map name for unofficial maps
    {
        g_pGameModeSystem->SetGameModeFromMapName(pMapName);
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

void CMapCache::OnFetchMapZones(KeyValues *pKv)
{
    const auto pData = pKv->FindKey("data");
    const auto pErr = pKv->FindKey("error");
    if (pData)
    {
        const auto pTracks = pData->FindKey("tracks");
        if (pTracks)
        {
            KeyValues *pToSend = new KeyValues("ZonesFromSite");
            // A copy is required, otherwise auto delete from pToSend will delete the content
            pToSend->SetPtr("tracks", pTracks->MakeCopy(true));
            engine->ServerCmdKeyValues(pToSend);
        }
        else
        {
            engine->ServerCmdKeyValues(new KeyValues("NoZones"));
        }
    }
    else if (pErr)
    {
        engine->ServerCmdKeyValues(new KeyValues("NoZones"));
        //MOM_TODO error handle here
    }
}

void CMapCache::ToggleMapLibraryOrFavorite(KeyValues* pKv, bool bIsLibrary, bool bAdded)
{
    KeyValues *pData = pKv->FindKey("data");
    KeyValues *pErr = pKv->FindKey("error");
    if (pData)
    {
        // Actually update the map in question, fire off an update
        const auto id = pData->GetInt("mapID");
        const auto pMapData = GetMapDataByID(id);
        if (pMapData)
        {
            (bIsLibrary ? pMapData->m_bInLibrary : pMapData->m_bInFavorites) = bAdded;
            if (bIsLibrary)
                pMapData->m_bMapFileNeedsUpdate = bAdded && !pMapData->m_bMapFileExists;
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
    m_pCurrentMapData = nullptr;

    const char *pMapName = pKv->GetString("map");

    const auto dictIndx = m_dictMapNames.Find(pMapName);
    if (m_dictMapNames.IsValidIndex(dictIndx))
    {
        const auto mapID = m_dictMapNames[dictIndx];
        const auto mapIndx = m_mapMapCache.Find(mapID);
        if (m_mapMapCache.IsValidIndex(mapIndx))
        {
            char hash[41];
            if (MomUtil::GetFileHash(hash, sizeof(hash), pKv->GetString("file")))
            {
                if (FStrEq(hash, m_mapMapCache[mapIndx]->m_szHash))
                    m_pCurrentMapData = m_mapMapCache[mapIndx];
            }

            // Check the update need & severity
            /*if (time(nullptr) - m_pCurrentMapData->m_tLastUpdateCheck > GetUpdateIntervalForMap(m_pCurrentMapData))
            {
                // Needs update
                DevLog("The map needs updating!\n");
                UpdateMapInfo(m_pCurrentMapData->m_uID);
            }*/
        }
    }

    SetMapGamemode(pMapName);

    const auto pEvent = gameeventmanager->CreateEvent("mapcache_map_load");
    if (pEvent)
    {
        pEvent->SetString("map", pMapName);
        gameeventmanager->FireEventClientSide(pEvent);
    }
}

void CMapCache::LevelInitPreEntity()
{
    if (g_pGameModeSystem->GameModeIs(GAMEMODE_TRICKSURF))
        return; // Tricksurf system handles getting zones and tricks

    if (m_pCurrentMapData && m_pCurrentMapData->m_bInLibrary)
    {
        g_pAPIRequests->GetMapZones(m_pCurrentMapData->m_uID, UtlMakeDelegate(this, &CMapCache::OnFetchMapZones));
    }
    else
    {
        engine->ServerCmdKeyValues(new KeyValues("NoZones"));
    }
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
    pMapData->UsesEscapeSequences(true);
    if (pMapData->LoadFromFile(g_pFullFileSystem, MAP_CACHE_FILE_NAME, "MOD"))
    {
        KeyValues *pVersion = pMapData->FindKey(MOM_CURRENT_VERSION);
        if (pVersion)
        {
            AddMapsToCache(pVersion, MODEL_FROM_DISK);
        }
        else
        {
            Log("Map cache file exists but is an older version, ignoring it...\n");
        }
    }
    else
    {
        Log("Map cache file doesn't exist, creating it...\n");
    }
}

void CMapCache::SaveMapCacheToDisk()
{
    KeyValuesAD pMapData("MapCacheData");
    pMapData->UsesEscapeSequences(true);

    KeyValues *pVersion = new KeyValues(MOM_CURRENT_VERSION);
    pMapData->AddSubKey(pVersion);

    FOR_EACH_MAP(m_mapMapCache, i)
    {
        KeyValues *pMap = pVersion->CreateNewKey();
        m_mapMapCache[i]->ToKV(pMap);
    }

    if (!pMapData->SaveToFile(g_pFullFileSystem, MAP_CACHE_FILE_NAME, "MOD"))
        DevLog("Failed to log map cache out to file\n");
}

CMapCache s_mapCache;
CMapCache *g_pMapCache = &s_mapCache;
