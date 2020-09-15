#include "cbase.h"

#include <vgui/ISurface.h>
#include <vgui/IInput.h>
#include "ContextMenu.h"

#include "tier0/memdbgon.h"

using namespace vgui;

ContextMenu::ContextMenu(Panel *parent) : Menu(parent, "ContextMenu")
{
}

void ContextMenu::ShowMenu()
{
    int x, y, gx, gy;
    input()->GetCursorPos(x, y);
    ipanel()->GetPos(surface()->GetEmbeddedPanel(), gx, gy);
    SetPos(x - gx - 5, y - gy - 5);
    SetVisible(true);
    SetMouseInputEnabled(true);
    MoveToFront();
}