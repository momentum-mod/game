#include "cbase.h"
#include "hud_menu_static.h"

#include "tier0/memdbgon.h"

static void OnClose()
{
    engine->ExecuteClientCmd("mom_saveloc_close");
}

CON_COMMAND(mom_saveloc_show, "Opens the Saved Locations Menu.\n")
{
    CHudMenuStatic *savelocMenu = GET_HUDELEMENT(CHudMenuStatic);
    if (savelocMenu)
    {
        if (savelocMenu->IsMenuDisplayed())
        {
            savelocMenu->HideMenu();//NOTE: if another menu is open this will close it!
        }
        else
        {
            savelocMenu->ShowMenu("savelocmenu", OnClose);
        }
    }
}