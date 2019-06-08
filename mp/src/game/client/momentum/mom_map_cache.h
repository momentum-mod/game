#pragma once

#include "mom_api_models.h"
#include "steam/isteamhttp.h"
#include "IMapList.h"

enum MapDownloadResponse
{
    MAP_DL_OK = 0,
    MAP_DL_FAIL,
    MAP_DL_ALREADY_EXISTS,
    MAP_DL_ALREADY_DOWNLOADING,
    MAP_DL_WILL_OVERWRITE_EXISTING,
};

class CMapCache : public CAutoGameSystem, public CGameEventListener
{
public:
    CMapCache();
    ~CMapCache();

    bool PlayMap(uint32 uID);
    bool MapFileExists(MapData *pData);
    MapDownloadResponse DownloadMap(uint32 uID, bool bOverwrite = false);
    bool CancelDownload(uint32 uID);

    bool AddMapToDeleteQueue(MapData *pData);
    bool RemoveMapFromDeleteQueue(MapData *pData);

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

    bool IsMapDownloading(uint32 uMapID);
    bool IsMapQueuedToDownload(uint32 uMapID) const;
    void OnDownloadQueueSizeChanged();
    void OnDownloadQueueToggled();
    void RemoveMapFromDownloadQueue(uint32 uMapID, bool bSendEvent = false);
protected:
    void PostInit() OVERRIDE;
    void PreLevelInit(KeyValues *pKv); // Called from server before Server's LevelInitPre/Post entity
    void LevelInitPreEntity() OVERRIDE;
    void LevelShutdownPostEntity() OVERRIDE;
    void Shutdown() OVERRIDE;

    void LoadMapCacheFromDisk();
    void SaveMapCacheToDisk();

    void SetMapGamemode(const char *pMapName = nullptr);

    // HTTP callbacks
    void OnFetchPlayerMapLibrary(KeyValues *pKv);
    void OnFetchPlayerMapFavorites(KeyValues *pKv);
    void OnFetchMapInfo(KeyValues *pKv);
    void OnFetchMapZones(KeyValues *pKv);

    void OnMapAddedToLibrary(KeyValues *pKv);
    void OnMapRemovedFromLibrary(KeyValues *pKv);
    void OnMapAddedToFavorites(KeyValues *pKv);
    void OnMapRemovedFromFavorites(KeyValues *pKv);

    // Map downloading
    void MapDownloadQueued(MapData *pData, bool bAdded);
    void MapDownloadStart(MapData *pData);
    void MapDownloadSize(KeyValues *pKvHeader);
    void MapDownloadProgress(KeyValues *pKvProgress);
    void MapDownloadEnd(KeyValues *pKvComplete);
private:
    void UpdateFetchedMaps(KeyValues *pKv, bool bIsLibrary);
    void ToggleMapLibraryOrFavorite(KeyValues *pKv, bool bIsLibrary, bool bAdded);
    bool StartDownloadingMap(MapData *pData);
    bool AddMapToDownloadQueue(MapData *pData);

    MapData *m_pCurrentMapData;

    CUtlDict<uint32> m_dictMapNames;
    CUtlMap<uint32, MapData*> m_mapMapCache;
    CUtlMap<uint32, MapData*> m_mapQueuedDelete;
    CUtlMap<uint32, MapData*> m_mapQueuedDownload;
    CUtlMap<HTTPRequestHandle, uint32> m_mapFileDownloads;
};

extern CMapCache* g_pMapCache;