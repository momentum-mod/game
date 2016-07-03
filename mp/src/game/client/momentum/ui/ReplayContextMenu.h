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
    CReplayContextMenu(vgui::Panel *parent);
    ~CReplayContextMenu();

    // call this to Activate the menu
    void ShowMenu( vgui::Panel *target, const char *runName);
};


#endif // REPLAYCONTEXTMENU_H