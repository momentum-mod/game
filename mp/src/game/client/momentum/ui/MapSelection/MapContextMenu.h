#pragma once

#include "vgui_controls/Menu.h"

struct MapData;
class CMapSelectorDialog;

//-----------------------------------------------------------------------------
// Purpose: Basic right-click context menu for servers
//-----------------------------------------------------------------------------
class CMapContextMenu : public vgui::Menu
{
public:
    CMapContextMenu(CMapSelectorDialog *parent);
    ~CMapContextMenu();

    // call this to Activate the menu
    void ShowMenu(MapData *pMapData);

private:
    CMapSelectorDialog *m_pParent;
};
