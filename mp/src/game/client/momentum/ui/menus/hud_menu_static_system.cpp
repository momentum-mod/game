#include "cbase.h"

#include "hud_menu_static_system.h"
#include "hud_menu_static.h"

#include "tier0/memdbgon.h"

CON_COMMAND(mom_hud_menu_hide, "Hides the currently open hud menu if there is one.\n")
{
    g_pHudMenuStaticSystem->HideMenu();
}

CON_COMMAND(mom_hud_menu_show, "Opens a hud menu. Accepts the hud menu itself by file name. See cfg/menus/", 0)
{
    if (args.ArgC() > 1)
    {
        g_pHudMenuStaticSystem->ShowMenu(args[1]);
    }
    else
    {
        Warning("This command expects a parameter that is the hud menu you want to open, by file name\n");
    }
}

CHudMenuStaticSystem::CHudMenuStaticSystem() : CAutoGameSystem("HudMenuStaticSystem")
{
}

void CHudMenuStaticSystem::ShowMenu(const char *pszFileName)
{
    CHudMenuStatic *pMenu = GET_HUDELEMENT(CHudMenuStatic);
    if (!pMenu)
        return;

    if (!pMenu->IsMenuDisplayed())
    {
        pMenu->ShowMenu(pszFileName);
        return;
    }

    bool bThisMenuDisplayed = pMenu->IsMenuDisplayed(pszFileName);
    pMenu->HideMenu();

    if (!bThisMenuDisplayed)
    {
        pMenu->ShowMenu(pszFileName);
    }
}

void CHudMenuStaticSystem::HideMenu() 
{
    CHudMenuStatic *pMenu = GET_HUDELEMENT(CHudMenuStatic);
    if (!pMenu)
        return;

    if (pMenu->IsMenuDisplayed())
    {
        pMenu->HideMenu();
    }
}

bool CHudMenuStaticSystem::IsMenuDisplayed(const char *pszMenuName)
{
    CHudMenuStatic *pMenu = GET_HUDELEMENT(CHudMenuStatic);
    if (!pMenu || !pszMenuName || !pszMenuName[0])
        return false;

    return pMenu->IsMenuDisplayed(pszMenuName);
}

static CHudMenuStaticSystem s_MenuSystem;
CHudMenuStaticSystem *g_pHudMenuStaticSystem = &s_MenuSystem;
