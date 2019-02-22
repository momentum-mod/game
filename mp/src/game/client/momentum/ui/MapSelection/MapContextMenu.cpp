#include "cbase.h"

#include "MapContextMenu.h"
#include "MapSelectorDialog.h"
#include "mom_map_cache.h"

#include "vgui/IInput.h"
#include "vgui/ISurface.h"

#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMapContextMenu::CMapContextMenu(CMapSelectorDialog *parent) : Menu(parent, "MapContextMenu"), m_pParent(parent)
{
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMapContextMenu::~CMapContextMenu()
{
}

//-----------------------------------------------------------------------------
// Purpose: Activates the menu
//-----------------------------------------------------------------------------
void CMapContextMenu::ShowMenu(MapData *pMapData)
{
    if (pMapData->m_bInLibrary)
    {
        if (pMapData->m_bMapFileNeedsUpdate)
        {
            if (m_pParent->IsMapDownloading(pMapData->m_uID))
                AddMenuItem("CancelDownload", "#MOM_MapSelector_CancelDownload", 
                            new KeyValues("CancelDownload", "id", pMapData->m_uID), m_pParent);
            else
                AddMenuItem("DownloadMap", "#MOM_MapSelector_DownloadMap",
                            new KeyValues("DownloadMap", "id", pMapData->m_uID), m_pParent);
        }
        else
        {
            AddMenuItem("StartMap", "#MOM_MapSelector_StartMap", 
                        new KeyValues("StartMap", "id", pMapData->m_uID), m_pParent);
        }

        AddMenuItem("RemoveFromLibrary", "#MOM_MapSelector_RemoveFromLibrary", 
                    new KeyValues("RemoveFromLibrary", "id", pMapData->m_uID), m_pParent);
    }
    else
    {
        AddMenuItem("AddToLibrary", "#MOM_MapSelector_AddToLibrary", 
                    new KeyValues("AddToLibrary", "id", pMapData->m_uID), m_pParent);
    }

    if (pMapData->m_bInFavorites)
    {
        AddMenuItem("RemoveFromFavorites", "#MOM_MapSelector_RemoveFromFavorites", 
                    new KeyValues("RemoveFromFavorites", "id", pMapData->m_uID), m_pParent);
    }
    else
    {
        AddMenuItem("AddToFavorites", "#MOM_MapSelector_AddToFavorites", 
                    new KeyValues("AddToFavorites", "id", pMapData->m_uID), m_pParent);
    }

    AddMenuItem("ViewMapInfo", "#MOM_MapSelector_ShowMapInfo", 
                new KeyValues("ViewMapInfo", "id", pMapData->m_uID), m_pParent);

    int x, y, gx, gy;
    input()->GetCursorPos(x, y);
    ipanel()->GetPos(surface()->GetEmbeddedPanel(), gx, gy);
    SetPos(x - gx, y - gy);
    SetVisible(true);
}