#include "cbase.h"

#include "MapContextMenu.h"

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
    bool showStart,
    bool showViewGameInfo)
{
    if (showStart)
    {
        AddMenuItem("StartMap", "#MOM_MapSelector_StartMap", new KeyValues("StartMap"), target);
    }

    if (showViewGameInfo)
    {
        AddMenuItem("ViewMapInfo", "#MOM_MapSelector_ShowMapInfo", new KeyValues("ViewMapInfo"), target);
    }

    int x, y, gx, gy;
    input()->GetCursorPos(x, y);
    ipanel()->GetPos(surface()->GetEmbeddedPanel(), gx, gy);
    SetPos(x - gx, y - gy);
    SetVisible(true);
}