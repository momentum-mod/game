#include "pch_mapselection.h"

using namespace vgui;

extern IFileSystem *filesystem;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CLocalMaps::CLocalMaps(vgui::Panel *parent) :
CBaseMapsPage(parent, "LocalMaps")
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
        GetWorkshopItems();
    }

    StartRefresh();
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the game list supports the specified ui elements
//-----------------------------------------------------------------------------
bool CLocalMaps::SupportsItem(InterfaceItem_e item)
{
    switch (item)
    {
    case FILTERS:
        return true;

    case GETNEWLIST:
    default:
        return false;
    }
}


bool MapHasStages(const char* szMap)
{
    bool found = false;
    if (filesystem && szMap)
    {
        KeyValues *kvMap = new KeyValues(szMap);
        char path[MAX_PATH];
        char fileName[FILENAME_MAX];
        Q_snprintf(fileName, FILENAME_MAX, "%s%s", szMap, EXT_ZONE_FILE);
        V_ComposeFileName(MAP_FOLDER, fileName, path, MAX_PATH);


        if (kvMap->LoadFromFile(filesystem, path, "MOD"))
        {
            found = (kvMap->FindKey("stage") != nullptr);
        }
        kvMap->deleteThis();
    }

    return found;
}

void CLocalMaps::FillMapstruct(mapstruct_t *m)
{
    //Game mode
    m->m_iGameMode = MOMGM_UNKNOWN;
    float tickRate = 0.015f;
    if (!V_strnicmp(m->m_szMapName, "surf_", 5))
    {
        m->m_iGameMode = MOMGM_SURF;
    }
    else if (!V_strnicmp(m->m_szMapName, "bhop_", 5))
    {
        m->m_iGameMode = MOMGM_BHOP;
        tickRate = 0.01f;
    }

    // MOM_TODO: Determine difficulty
    m->m_iDifficulty = 1;

    //Map layout (liner/staged)
    m->m_bHasStages = MapHasStages(m->m_szMapName);

    //Completed/Best time
    //MOM_TODO: have the tickrate and run flags as filters, load actual values
    
    CMomReplayBase *pBestTime = g_pMomentumUtil->GetBestTime(m->m_szMapName, tickRate);
    if (pBestTime)
    {
        m->m_bCompleted = true;
        Log("FOUND BEST TIME: %f\n", pBestTime->GetRunTime());
        g_pMomentumUtil->FormatTime(pBestTime->GetRunTime(), m->m_szBestTime);
    }
}

void CLocalMaps::GetNewMapList()
{
    ClearMapList();
    //Populate the main list
    FileFindHandle_t found;
    //MOM_TODO: make this by *.mom
    const char *pMapName = g_pFullFileSystem->FindFirstEx("maps/*.bsp", "GAME", &found);
    while (pMapName)
    {       
        AddNewMapToVector(pMapName);
        pMapName = g_pFullFileSystem->FindNext(found);
    }
    g_pFullFileSystem->FindClose(found);

    m_bLoadedMaps = true;

    ApplyGameFilters();
}

void CLocalMaps::AddNewMapToVector(const char* mapname)
{
    mapdisplay_t map = mapdisplay_t();
    mapstruct_t m = mapstruct_t();
    map.m_bDoNotRefresh = true;

    //Map name
    V_FileBase(mapname, m.m_szMapName, MAX_PATH);
    FillMapstruct(&m);

    // Map image
    if (g_pMomentumUtil->MapThumbnailExists(m.m_szMapName))
    {
        DevLog("FOUND IMAGE FOR %s!\n", m.m_szMapName);
        char imagePath[MAX_PATH];
        Q_snprintf(imagePath, MAX_PATH, "maps/%s", m.m_szMapName);
        map.m_iMapImageIndex = m_pMapList->GetImageList()->AddImage(scheme()->GetImage(imagePath, false));
    }

    map.m_mMap = m;
    m_vecMaps.AddToTail(map);
}

//-----------------------------------------------------------------------------
// Purpose: starts the maps refreshing
//-----------------------------------------------------------------------------
void CLocalMaps::StartRefresh()
{
    FOR_EACH_VEC(m_vecMaps, i)
    {
        mapdisplay_t *pMap = &m_vecMaps[i];
        if (!pMap) continue;
        mapstruct_t pMapInfo = pMap->m_mMap;
        // check filters
        bool removeItem = false;
        if (!CheckPrimaryFilters(pMapInfo))
        {
            // map has been filtered at a primary level
            // remove from lists
            pMap->m_bDoNotRefresh = true;

            // remove from UI list
            removeItem = true;
        }
        else if (!CheckSecondaryFilters(pMapInfo))
        {
            // we still ping this server in the future; however it is removed from UI list
            removeItem = true;
        }

        if (removeItem)
        {
            if (m_pMapList->IsValidItemID(pMap->m_iListID))
            {
                m_pMapList->RemoveItem(pMap->m_iListID);
                pMap->m_iListID = GetInvalidMapListID();
            }
            continue;
        }

        // update UI
        KeyValues *kv;
        if (m_pMapList->IsValidItemID(pMap->m_iListID))
        {
            // we're updating an existing entry
            kv = m_pMapList->GetItem(pMap->m_iListID);
        }
        else
        {
            // new entry
            kv = new KeyValues("Map");
        }
        
        kv->SetString(KEYNAME_MAP_NAME, pMapInfo.m_szMapName);
        kv->SetString(KEYNAME_MAP_LAYOUT, pMapInfo.m_bHasStages ? "STAGED" : "LINEAR");
        kv->SetInt(KEYNAME_MAP_DIFFICULTY, pMapInfo.m_iDifficulty);
        kv->SetString(KEYNAME_MAP_BEST_TIME, pMapInfo.m_szBestTime);
        kv->SetInt(KEYNAME_MAP_IMAGE, pMap->m_iMapImageIndex);
        
        if (!m_pMapList->IsValidItemID(pMap->m_iListID))
        {
            // new map, add to list
            pMap->m_iListID = m_pMapList->AddItem(kv, NULL, false, false);
            if (m_bAutoSelectFirstItemInGameList && m_pMapList->GetItemCount() == 1)
            {
                m_pMapList->AddSelectedItem(pMap->m_iListID);
            }

            kv->deleteThis();
        }
        else
        {
            // tell the list that we've changed the data
            m_pMapList->ApplyItemChanges(pMap->m_iListID);
            m_pMapList->SetItemVisible(pMap->m_iListID, true);
        }
    }
}


//-----------------------------------------------------------------------------
// Purpose: Control which button are visible.
//-----------------------------------------------------------------------------
void CLocalMaps::ManualShowButtons(bool bShowConnect, bool bShowRefreshAll, bool bShowFilter)
{
    m_pStartMap->SetVisible(bShowConnect);
    m_pQueryMaps->SetVisible(bShowRefreshAll);
    m_pFilter->SetVisible(bShowFilter);
}

void CLocalMaps::SetEmptyListText()
{
    m_pMapList->SetEmptyListText("#MOM_MapSelector_NoMaps");
}

void CLocalMaps::GetWorkshopItems()
{
    //get a vector of all the item handles
    const uint32 numItems = steamapicontext->SteamUGC()->GetNumSubscribedItems();
    const auto vecItems = new PublishedFileId_t[numItems];
    steamapicontext->SteamUGC()->GetSubscribedItems(vecItems, numItems);

    //check them all
    for(uint32 i = 0; i < numItems; ++i)
    {
        const auto currentItem = vecItems[i];
        const uint32 flags = steamapicontext->SteamUGC()->GetItemState(currentItem);
        if (!(flags & k_EItemStateSubscribed)) //we're not subscribed to this item - how did this even happen?
            continue;
            
        if (!(flags & k_EItemStateInstalled) || flags & k_EItemStateNeedsUpdate) //we're subscribed, but item is not installed, OR item requires an update
        {
            const auto call = steamapicontext->SteamUGC()->DownloadItem(currentItem, true); 
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
    if (!steamapicontext->SteamUGC()->GetItemInstallInfo(id, &sizeOnDisk, szFolder, sizeof(szFolder), &timeStamp))
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
}

//-----------------------------------------------------------------------------
// Purpose: opens context menu (user right clicked on a map)
//-----------------------------------------------------------------------------
void CLocalMaps::OnOpenContextMenu(int row)
{
    if (!m_pMapList->GetSelectedItemsCount())
        return;

    // Activate context menu
    CMapContextMenu *menu = MapSelectorDialog().GetContextMenu(m_pMapList);
    menu->ShowMenu(this, true, true);
}