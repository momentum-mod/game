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
        GetNewMapList();

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
        Q_snprintf(fileName, FILENAME_MAX, "%s%s", szMap, EXT_TIME_FILE);
        V_ComposeFileName(MAP_FOLDER, fileName, path, MAX_PATH);


        if (kvMap->LoadFromFile(filesystem, path, "MOD"))
        {
            found = (kvMap->FindKey("zone") != nullptr);
        }
        kvMap->deleteThis();
    }

    return found;
}

void CLocalMaps::FillMapstruct(mapstruct_t *m)
{
    //Game mode
    m->m_iGameMode = MOMGM_UNKNOWN;
    float tickRate = 1.0f/66.0f;
    if (!Q_strnicmp(m->m_szMapName, "surf_", 5))
    {
        m->m_iGameMode = MOMGM_SURF;
    }
    else if (!Q_strnicmp(m->m_szMapName, "bhop_", 5))
    {
        m->m_iGameMode = MOMGM_BHOP;
        tickRate = 0.01f;
    }

    // MOM_TODO: Determine difficulty
    m->m_iDifficulty = 1;

    //Map layout (liner/staged)
    m->m_bHasStages = MapHasStages(m->m_szMapName);

    //Completed/Best time
    KeyValues *kvMapWrapper = new KeyValues(m->m_szMapName);
    //MOM_TODO: have the tickrate and run flags as filters, load actual values
    
    KeyValues *kvMapTime = mom_UTIL->GetBestTime(kvMapWrapper, m->m_szMapName, tickRate);
    if (kvMapTime)
    {
        m->m_bCompleted = true;
        mom_UTIL->FormatTime(Q_atof(kvMapTime->GetName()), m->m_szBestTime);
    }
    kvMapWrapper->deleteThis();
}


void CLocalMaps::GetNewMapList()
{
    ClearMapList();
    //Populate the main list
    FileFindHandle_t found;
    //MOM_TODO: make this by *.mom
    const char *pMapName = g_pFullFileSystem->FindFirstEx("maps/*.bsp", "MOD", &found);
    while (pMapName)
    {       
        DevLog("FOUND MAP %s!\n", pMapName);
        
        mapdisplay_t map = mapdisplay_t();
        mapstruct_t m = mapstruct_t();
        map.m_bDoNotRefresh = true;

        //Map name
        Q_FileBase(pMapName, m.m_szMapName, MAX_PATH);
        DevLog("Stripped name: %s\n", m.m_szMapName);

        FillMapstruct(&m);

        map.m_mMap = m;
        m_vecMaps.AddToTail(map);

        pMapName = g_pFullFileSystem->FindNext(found);
    }
    g_pFullFileSystem->FindClose(found);

    ApplyGameFilters();
}

//-----------------------------------------------------------------------------
// Purpose: starts the maps refreshing
//-----------------------------------------------------------------------------
void CLocalMaps::StartRefresh()
{
    FOR_EACH_VEC(m_vecMaps, i)
    {
        mapdisplay_t *pMap = &m_vecMaps[0];
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
            if (m_pGameList->IsValidItemID(pMap->m_iListID))
            {
                m_pGameList->RemoveItem(pMap->m_iListID);
                pMap->m_iListID = GetInvalidMapListID();
            }
            return;
        }

        // update UI
        KeyValues *kv;
        if (m_pGameList->IsValidItemID(pMap->m_iListID))
        {
            // we're updating an existing entry
            kv = m_pGameList->GetItem(pMap->m_iListID);
        }
        else
        {
            // new entry
            kv = new KeyValues("Map");
        }

        kv->SetString("name", pMapInfo.m_szMapName);
        kv->SetInt("MapLayout", ((int)pMapInfo.m_bHasStages) + 2);//+ 2 so the picture sets correctly
        kv->SetBool("completed", pMapInfo.m_bCompleted);
        kv->SetInt("difficulty", pMapInfo.m_iDifficulty);
        kv->SetInt("gamemode", pMapInfo.m_iGameMode);
        kv->SetString("time", pMapInfo.m_szBestTime);

        if (!m_pGameList->IsValidItemID(pMap->m_iListID))
        {
            // new map, add to list
            pMap->m_iListID = m_pGameList->AddItem(kv, NULL, false, false);
            if (m_bAutoSelectFirstItemInGameList && m_pGameList->GetItemCount() == 1)
            {
                m_pGameList->AddSelectedItem(pMap->m_iListID);
            }

            kv->deleteThis();
        }
        else
        {
            // tell the list that we've changed the data
            m_pGameList->ApplyItemChanges(pMap->m_iListID);
            m_pGameList->SetItemVisible(pMap->m_iListID, true);
        }
    }
}


//-----------------------------------------------------------------------------
// Purpose: Control which button are visible.
//-----------------------------------------------------------------------------
void CLocalMaps::ManualShowButtons(bool bShowConnect, bool bShowRefreshAll, bool bShowFilter)
{
    m_pStartMap->SetVisible(bShowConnect);
    m_pRefreshAll->SetVisible(bShowRefreshAll);
    m_pFilter->SetVisible(bShowFilter);
}

void CLocalMaps::SetEmptyListText()
{
    m_pGameList->SetEmptyListText("#MOM_MapSelector_NoMaps");
}

//-----------------------------------------------------------------------------
// Purpose: opens context menu (user right clicked on a map)
//-----------------------------------------------------------------------------
void CLocalMaps::OnOpenContextMenu(int row)
{
    if (!m_pGameList->GetSelectedItemsCount())
        return;

    // Activate context menu
    CMapContextMenu *menu = MapSelectorDialog().GetContextMenu(m_pGameList);
    menu->ShowMenu(this, true, true);
}