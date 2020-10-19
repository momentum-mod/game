#include "cbase.h"

#include "hud_menu_static_system.h"
#include "hud_menu_static.h"
#include "filesystem.h"

#include "tier0/memdbgon.h"

CON_COMMAND(mom_hud_menu_reload, "Reloads the menu listing. These are located in cfg/menus/\n")
{
    g_pHudMenuStaticSystem->ReloadMenuList();
}

CON_COMMAND(mom_hud_menu_hide, "Hides the currently open hud menu if there is one.\n")
{
    g_pHudMenuStaticSystem->HideMenu();
}

static int HudMenuCompletion(const char *pPartial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
    const auto pCmdName = "mom_hud_menu_show";
    char *pSubstring = nullptr;
    if (Q_strstr(pPartial, pCmdName) && strlen(pPartial) > strlen(pCmdName) + 1)
    {
        pSubstring = (char *)pPartial + strlen(pCmdName) + 1;
    }

    const bool bSubstringNullOrEmpty = !pSubstring || !pSubstring[0];
    int current = 0;

    for (int i = 0; i < g_pHudMenuStaticSystem->GetNumMenus(); i++)
    {
        const char *menuName = g_pHudMenuStaticSystem->GetMenuName(i);

        if (bSubstringNullOrEmpty || Q_stristr(menuName, pSubstring))
        {
            char command[COMMAND_COMPLETION_ITEM_LENGTH];
            Q_snprintf(command, COMMAND_COMPLETION_ITEM_LENGTH, "%s %s", pCmdName, menuName);
            Q_strncpy(commands[current], command, COMMAND_COMPLETION_ITEM_LENGTH);
            current++;
        }
    }

    return current;
}

CON_COMMAND_F_COMPLETION(mom_hud_menu_show, "Opens a hud menu. Accepts the hud menu itself by file name. See cfg/menus/\n", 0, HudMenuCompletion)
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

CHudMenuStaticSystem::CHudMenuStaticSystem() : CAutoGameSystem("HudMenuStaticSystem") {}

void CHudMenuStaticSystem::PostInit()
{
    ReloadMenuList();
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

void CHudMenuStaticSystem::ReloadMenuList()
{
    m_vecMenuNames.RemoveAll();

    FileFindHandle_t findHandle;
    const char *pFilename = g_pFullFileSystem->FindFirstEx("cfg/menus/*.vdf", "MOD", &findHandle);
    char pPrevFilename[64];
    while (pFilename)
    {
        char pszFilenameNoExt[64], pszFilenameNoDefault[64];
        Q_StripExtension(pFilename, pszFilenameNoExt, 64);
        Q_StrSubst(pszFilenameNoExt, "_default", "", pszFilenameNoDefault, 64);

        if (!FStrEq(pPrevFilename, pszFilenameNoDefault)) // didn't just parse non-default file
            m_vecMenuNames.CopyAndAddToTail(pszFilenameNoDefault);

        Q_strncpy(pPrevFilename, pszFilenameNoDefault, 64);
        pFilename = g_pFullFileSystem->FindNext(findHandle);
    }

    g_pFullFileSystem->FindClose(findHandle);
}

static CHudMenuStaticSystem s_MenuSystem;
CHudMenuStaticSystem *g_pHudMenuStaticSystem = &s_MenuSystem;
