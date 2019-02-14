#include "cbase.h"

#include "FavoriteMaps.h"

#include "CMapListPanel.h"
#include "mom_map_cache.h"

#include "tier0/memdbgon.h"


using namespace vgui;

CFavoriteMaps::CFavoriteMaps(Panel* pParent): CBaseMapsPage(pParent, "FavoriteMaps")
{
    m_bLoadedMaps = false;
    m_pMapList->SetColumnVisible(HEADER_MAP_IN_FAVORITES, false);
}

void CFavoriteMaps::OnTabSelected()
{
    if (!m_bLoadedMaps)
    {
        GetNewMapList();
    }
}

void CFavoriteMaps::OnGetNewMapList()
{
    m_bLoadedMaps = !m_vecMaps.IsEmpty();

    BaseClass::OnGetNewMapList();
}

void CFavoriteMaps::FireGameEvent(IGameEvent* event)
{
    if (FStrEq(event->GetName(), "map_cache_updated"))
    {
        if (event->GetInt("source") == MODEL_FROM_FAVORITES_API_CALL)
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
                // Remove this map only if we have it and it's no longer in favorites
                if (pDisplay->m_pMap && !pDisplay->m_pMap->m_bInFavorites)
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
                    if (pMapData->m_bInFavorites)
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