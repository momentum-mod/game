#pragma once

#include <vgui_controls/Menu.h>

class ContextMenu : public vgui::Menu
{
public:
    DECLARE_CLASS_SIMPLE(ContextMenu, vgui::Menu);
    ContextMenu(Panel *parent);

    // call this to Activate the menu
    void ShowMenu();
};