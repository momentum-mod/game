#ifndef REPLAYCONTEXTMENU_H
#define REPLAYCONTEXTMENU_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include <vgui_controls/Menu.h>

//-----------------------------------------------------------------------------
// Purpose: Basic right-click context menu for servers
//-----------------------------------------------------------------------------
class CReplayContextMenu : public vgui::Menu
{
public:
    DECLARE_CLASS_SIMPLE(CReplayContextMenu, vgui::Menu);
    CReplayContextMenu(Panel *parent);
    ~CReplayContextMenu();

    // call this to Activate the menu
    void ShowMenu();
    void OnCursorExitedMenuItem(int vpanel) override;
};


#endif // REPLAYCONTEXTMENU_H