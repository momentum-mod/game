#include "cbase.h"

#include "LibraryMaps.h"
#include "CMapListPanel.h"
#include "MapSelectorDialog.h"

#include "mom_map_cache.h"

#include "vgui_controls/Panel.h"

#include "tier0/memdbgon.h"

using namespace vgui;

extern ConVar mom_map_download_auto;
extern ConVar mom_map_delete_queue;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CLibraryMaps::CLibraryMaps(Panel *parent) : CBaseMapsPage(parent, "LibraryMaps")
{
    m_bLoadedMaps = false;
    m_pMapList->SetColumnVisible(HEADER_MAP_IN_LIBRARY, false);
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
                if (pMapData->m_bMapFileNeedsUpdate &&
                    (mom_map_download_auto.GetBool() || MapSelectorDialog().GetMapToStart() == pMapData->m_uID) &&
                    !MapSelectorDialog().IsMapDownloading(id))
                {
                    MapSelectorDialog().OnStartMapDownload(id);
                }
            }
            else
            {
                // Remove this map only if we have it and it's no longer in the library
                if (pMapData->m_bMapFileExists)
                {
                    if (mom_map_delete_queue.GetBool())
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
                if (!mom_map_download_auto.GetBool())
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
    }
}