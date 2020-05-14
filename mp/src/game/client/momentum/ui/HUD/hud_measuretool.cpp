#include "cbase.h"
#include "hud_menu_static.h"
#include "mom_player_shared.h"

#include "tier0/memdbgon.h"

static void OnClose()
{
    engine->ExecuteClientCmd("mom_ruler_close");
}

CON_COMMAND(showRuler, "Opens the ruler tool.\n")
{
    CHudMenuStatic *rulerMenu = GET_HUDELEMENT(CHudMenuStatic);
    if (rulerMenu && engine->IsInGame())
    {
        if (rulerMenu->IsMenuDisplayed())
        {
            rulerMenu->HideMenu();//NOTE: if another menu is open this will close it!
        }
        else
        {
            rulerMenu->ShowMenu("rulermenu", OnClose);
        }
    }
}