#pragma once

#include "cbase.h"
#include <vgui_controls/Menu.h>

//-----------------------------------------------------------------------------
// Purpose: Basic right-click context menu for servers
//-----------------------------------------------------------------------------
class CLeaderboardsContextMenu : public vgui::Menu
{
public:
    DECLARE_CLASS_SIMPLE(CLeaderboardsContextMenu, vgui::Menu);
    CLeaderboardsContextMenu(Panel *parent);
    ~CLeaderboardsContextMenu();

    // call this to Activate the menu
    void ShowMenu();
    void OnCursorExitedMenuItem(int vpanel) OVERRIDE;
};