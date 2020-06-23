#pragma once

#include <vgui_controls/Menu.h>

class CLeaderboardsContextMenu : public vgui::Menu
{
public:
    DECLARE_CLASS_SIMPLE(CLeaderboardsContextMenu, vgui::Menu);
    CLeaderboardsContextMenu(Panel *parent);

    // call this to Activate the menu
    void ShowMenu();
};