#include "cbase.h"

#include "MapContextMenu.h"
#include "mom_map_cache.h"

#include "vgui/IInput.h"
#include "vgui/ISurface.h"

#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMapContextMenu::CMapContextMenu(Panel *parent) : Menu(parent, "MapContextMenu")
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
void CMapContextMenu::ShowMenu(
    Panel *target,
    MapData *pMapData)
{
    if (pMapData->m_bInLibrary)
    {
        AddMenuItem("StartMap", "#MOM_MapSelector_StartMap", 
                    new KeyValues("StartMap", "id", pMapData->m_uID), target);
        AddMenuItem("RemoveFromLibrary", "#MOM_MapSelector_RemoveFromLibrary", 
                    new KeyValues("RemoveFromLibrary", "id", pMapData->m_uID), target);
    }
    else
    {
        AddMenuItem("AddToLibrary", "#MOM_MapSelector_AddToLibrary", 
                    new KeyValues("AddToLibrary", "id", pMapData->m_uID), target);
    }

    if (pMapData->m_bInFavorites)
    {
        AddMenuItem("RemoveFromFavorites", "#MOM_MapSelector_RemoveFromFavorites", 
                    new KeyValues("RemoveFromFavorites", "id", pMapData->m_uID), target);
    }
    else
    {
        AddMenuItem("AddToFavorites", "#MOM_MapSelector_AddToFavorites", 
                    new KeyValues("AddToFavorites", "id", pMapData->m_uID), target);
    }

    AddMenuItem("ViewMapInfo", "#MOM_MapSelector_ShowMapInfo", 
                new KeyValues("ViewMapInfo", "id", pMapData->m_uID), target);

    int x, y, gx, gy;
    input()->GetCursorPos(x, y);
    ipanel()->GetPos(surface()->GetEmbeddedPanel(), gx, gy);
    SetPos(x - gx, y - gy);
    SetVisible(true);
}