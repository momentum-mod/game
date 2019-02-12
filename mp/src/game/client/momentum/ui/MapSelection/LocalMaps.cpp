#include "cbase.h"

#include "LocalMaps.h"
#include "CMapListPanel.h"
#include "MapContextMenu.h"
#include "MapSelectorDialog.h"

#include "filesystem.h"
#include "mom_map_cache.h"

#include "vgui_controls/Panel.h"
#include "vgui_controls/ImageList.h"

#include "tier0/memdbgon.h"


using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CLocalMaps::CLocalMaps(Panel *parent) : CBaseMapsPage(parent, "LocalMaps")
{
    m_bLoadedMaps = false;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CLocalMaps::~CLocalMaps()
{
}


//-----------------------------------------------------------------------------
// Purpose: Activates the page, starts refresh
//-----------------------------------------------------------------------------
void CLocalMaps::OnPageShow()
{
    if (!m_bLoadedMaps)
    {
        GetNewMapList();
        // GetWorkshopItems();
    }
}

void CLocalMaps::GetNewMapList()
{
    // ClearMapList();
    //Populate the main list
    CUtlVector<MapData*> vecLibrary;
    g_pMapCache->GetMapList(vecLibrary, MAP_LIST_LIBRARY);

    FOR_EACH_VEC(vecLibrary, i)
        AddMapToList(vecLibrary[i]);

    m_bLoadedMaps = m_vecMaps.IsEmpty();

    ApplyFilters(GetFilters());
}

void CLocalMaps::SetEmptyListText()
{
    m_pMapList->SetEmptyListText("#MOM_MapSelector_NoMaps");
}

void CLocalMaps::FireGameEvent(IGameEvent* event)
{
    if (FStrEq(event->GetName(), "map_cache_updated"))
    {
        if (event->GetInt("source") == MODEL_FROM_LIBRARY_API_CALL)
            GetNewMapList();
    }
    else if (FStrEq(event->GetName(), "map_data_update"))
    {
        if (event->GetBool("main"))
        {
            int id = event->GetInt("id");
            MapDisplay_t *pDisplay = GetMapDisplayByID(id);
            
            if (pDisplay)
            {
                // Remove this map only if we have it and it's no longer in the library
                if (pDisplay->m_pMap && !pDisplay->m_pMap->m_bInLibrary)
                {
                    RemoveMap(*pDisplay);
                    return;
                }
            }
            else
            {
                MapData *pMapData = g_pMapCache->GetMapDataByID(id);
                if (pMapData)
                {
                    // Add this map if it was added to library
                    if (pMapData->m_bInLibrary)
                    {
                        AddMapToList(pMapData);
                        return;
                    }
                }
            }
        }
    }

    BaseClass::FireGameEvent(event);
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