#pragma once

#include "vgui_controls/Menu.h"

struct MapData;

//-----------------------------------------------------------------------------
// Purpose: Basic right-click context menu for servers
//-----------------------------------------------------------------------------
class CMapContextMenu : public vgui::Menu
{
public:
    CMapContextMenu(vgui::Panel *parent);
    ~CMapContextMenu();

    // call this to Activate the menu
    void ShowMenu(
        Panel *target,
        MapData *pMapData);
};
