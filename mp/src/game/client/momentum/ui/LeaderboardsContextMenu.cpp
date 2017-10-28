#include "cbase.h"
#include <vgui/ISurface.h>
#include <vgui/IInput.h>
#include "LeaderboardsContextMenu.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CLeaderboardsContextMenu::CLeaderboardsContextMenu(Panel *parent) : Menu(parent, "LeaderboardsContextMenu")
{
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CLeaderboardsContextMenu::~CLeaderboardsContextMenu()
{
}

//-----------------------------------------------------------------------------
// Purpose: Activates the menu
//-----------------------------------------------------------------------------
void CLeaderboardsContextMenu::ShowMenu()
{
    int x, y, gx, gy;
    input()->GetCursorPos(x, y);
    ipanel()->GetPos(surface()->GetEmbeddedPanel(), gx, gy);
    SetPos(x - gx - 5, y - gy - 5);
    SetVisible(true);
    SetMouseInputEnabled(true);
}

void CLeaderboardsContextMenu::OnCursorExitedMenuItem(int vpanel)
{ 
    int x, y;
    input()->GetCursorPosition(x, y);
    bool inside = IsWithin(x, y);
    SetVisible(inside && IsVisible());
    SetMouseInputEnabled(inside && IsVisible());
    BaseClass::OnCursorExitedMenuItem(vpanel);
}
