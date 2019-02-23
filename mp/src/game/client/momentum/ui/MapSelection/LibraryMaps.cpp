#include "cbase.h"

#include "LibraryMaps.h"
#include "CMapListPanel.h"
#include "MapSelectorDialog.h"

#include "mom_map_cache.h"
#include "mom_modulecomms.h"

#include "vgui_controls/Panel.h"

#include "tier0/memdbgon.h"


using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CLibraryMaps::CLibraryMaps(Panel *parent) : CBaseMapsPage(parent, "LibraryMaps"), m_cvarAutoDownload("mom_map_download_auto"),
    m_cvarDeleteQueue("mom_map_delete_queue")
{
    m_bLoadedMaps = false;
    m_pMapList->SetColumnVisible(HEADER_MAP_IN_LIBRARY, false);

    g_pModuleComms->ListenForEvent("map_cache_updated", UtlMakeDelegate(this, &CLibraryMaps::OnMapCacheUpdated));
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CLibraryMaps::~CLibraryMaps()
{
}

void CLibraryMaps::OnGetNewMapList()
{
    m_bLoadedMaps = m_mapMaps.Count() > 0;

    BaseClass::OnGetNewMapList();
}

void CLibraryMaps::OnMapCacheUpdated(KeyValues* pKv)
{
    if (pKv->GetInt("source") == MODEL_FROM_LIBRARY_API_CALL)
        GetNewMapList();
}

void CLibraryMaps::OnMapListDataUpdate(int id)
{
    MapDisplay_t *pDisplay = GetMapDisplayByID(id);
    if (pDisplay)
    {
        MapData *pMapData = pDisplay->m_pMap;
        if (pMapData)
        {
            if (pMapData->m_bInLibrary)
            {
                // Check to see if we should download
                if (pMapData->m_bMapFileNeedsUpdate && m_cvarAutoDownload.GetBool() && !MapSelectorDialog().IsMapDownloading(id))
                {
                    g_pMapCache->DownloadMap(id);
                }
            }
            else
            {
                // Remove this map only if we have it and it's no longer in the library
                if (pMapData->m_bMapFileExists)
                {
                    if (m_cvarDeleteQueue.GetBool())
                    {
                        g_pMapCache->AddMapToDeleteQueue(pMapData);
                    }
                    else
                    {
                        // Delete the file now
                        pMapData->DeleteMapFile();
                    }
                }

                if (g_pMapCache->IsMapDownloading(pMapData->m_uID))
                {
                    g_pMapCache->CancelDownload(pMapData->m_uID);
                }
                else if (g_pMapCache->IsMapQueuedToDownload(pMapData->m_uID))
                {
                    g_pMapCache->RemoveMapFromDownloadQueue(pMapData->m_uID);
                }

                RemoveMap(*pDisplay);

                pMapData->m_bMapFileNeedsUpdate = false;
                pMapData->m_bUpdated = true;
                pMapData->SendDataUpdate();
                return;
            }
        }
    }
    else
    {
        MapData *pMapData = g_pMapCache->GetMapDataByID(id);
        if (pMapData && pMapData->m_bInLibrary)
        {
            if (pMapData->m_bMapFileExists)
            {
                // MOM_TODO also check if the map file was updated if the map is in testing

                g_pMapCache->RemoveMapFromDeleteQueue(pMapData);
            }
            else
            {
                // We need the map, force this true
                pMapData->m_bMapFileNeedsUpdate = true;
            }

            // Add this map if it was added to library
            AddMapToList(pMapData);

            if (pMapData->m_bMapFileNeedsUpdate)
            {
                if (!m_cvarAutoDownload.GetBool())
                {
                    pMapData->m_bUpdated = true;
                    pMapData->SendDataUpdate();
                }
            }

            return;
        }
    }

    BaseClass::OnMapListDataUpdate(id);
}

void CLibraryMaps::OnTabSelected()
{
    if (!m_bLoadedMaps)
    {
        GetNewMapList();
        // GetWorkshopItems();
    }
}

/*void CLocalMaps::GetWorkshopItems()
{
    //get a vector of all the item handles
    const uint32 numItems = SteamUGC()->GetNumSubscribedItems();
    const auto vecItems = new PublishedFileId_t[numItems];
    SteamUGC()->GetSubscribedItems(vecItems, numItems);

    //check them all
    for(uint32 i = 0; i < numItems; ++i)
    {
        const auto currentItem = vecItems[i];
        const uint32 flags = SteamUGC()->GetItemState(currentItem);
        if (!(flags & k_EItemStateSubscribed)) //we're not subscribed to this item - how did this even happen?
            continue;
            
        if (!(flags & k_EItemStateInstalled) || flags & k_EItemStateNeedsUpdate) //we're subscribed, but item is not installed, OR item requires an update
        {
            const auto call = SteamUGC()->DownloadItem(currentItem, true); 
            if (call)
            {
                m_DownloadCompleteCallback.Set(call, this, &CLocalMaps::OnWorkshopDownloadComplete);
            }
        }
        else if (flags & k_EItemStateInstalled)
        {
            //item is already installed, lets add it to the list!
            AddWorkshopItemToLocalMaps(currentItem);
        }
    }
    delete[] vecItems;
}

//asynchronous call result for workshop item download
void CLocalMaps::OnWorkshopDownloadComplete(DownloadItemResult_t* pCallback, bool bIOFailure)
{
    if (bIOFailure || !(pCallback->m_eResult & k_EResultOK ))
    {
        Warning("Steam workshop item failed to download! ID: %llu Error code: %d",
            pCallback->m_nPublishedFileId,
            pCallback->m_eResult);
        return;
    }
    AddWorkshopItemToLocalMaps(pCallback->m_nPublishedFileId);
}

void CLocalMaps::AddWorkshopItemToLocalMaps(PublishedFileId_t id)
{
    //get info about the item from SteamUGC
    uint64 sizeOnDisk;
    char szFolder[MAX_PATH];
    uint32 timeStamp;
    if (!SteamUGC()->GetItemInstallInfo(id, &sizeOnDisk, szFolder, sizeof(szFolder), &timeStamp))
    {
        Warning("Could not get content for workshop item %llu. The item has no content or is not installed!\n", id);
        return;
    }

    //find the first bsp file in the workshop folder
    FileFindHandle_t found;
    char pOutPath[MAX_PATH];
    V_ComposeFileName(szFolder, "maps/*.bsp", pOutPath, MAX_PATH);
    const char *pMapName = g_pFullFileSystem->FindFirst(pOutPath, &found);

    while (pMapName) //add multiple maps if the workshop item contained more than one
    {
        g_pFullFileSystem->AddSearchPath(szFolder, "GAME");
        AddNewMapToVector(pMapName);
        //MOM_TODO: add thumbnail images? 

        pMapName = g_pFullFileSystem->FindNext(found); 
    }

    //now our workshop items are added, refresh the map list again
    StartRefresh();
}*/
