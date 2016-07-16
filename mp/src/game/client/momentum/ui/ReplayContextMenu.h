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
    CReplayContextMenu(vgui::Panel *parent);
    ~CReplayContextMenu();

    // call this to Activate the menu
    void ShowMenu();
    void OnCursorExitedMenuItem(int vpanel) override;
    // This one works weird, so using OnCurosExitedMenuItem until we figure how it works
    //void OnCursorExited() override;
};


#endif // REPLAYCONTEXTMENU_H