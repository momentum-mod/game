#include "cbase.h"

#include <vgui/ISurface.h>
#include <vgui/IInput.h>
#include "LeaderboardsContextMenu.h"

#include "tier0/memdbgon.h"

using namespace vgui;

CLeaderboardsContextMenu::CLeaderboardsContextMenu(Panel *parent) : Menu(parent, "LeaderboardsContextMenu")
{
}

void CLeaderboardsContextMenu::ShowMenu()
{
    int x, y, gx, gy;
    input()->GetCursorPos(x, y);
    ipanel()->GetPos(surface()->GetEmbeddedPanel(), gx, gy);
    SetPos(x - gx - 5, y - gy - 5);
    SetVisible(true);
    SetMouseInputEnabled(true);
    MoveToFront();
}