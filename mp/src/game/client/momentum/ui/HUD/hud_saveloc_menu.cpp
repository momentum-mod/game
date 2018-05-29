#include "cbase.h"
#include "hud_menu_static.h"

#include "tier0/memdbgon.h"

static void SelectMenuItem(int menu_item)
{
    switch (menu_item)
    {
    case 1://create a saveloc
        engine->ExecuteClientCmd("mom_saveloc_create");
        break;
    case 2://load previous saveloc
        engine->ExecuteClientCmd("mom_saveloc_current");
        break;
    case 3://cycle through savelocs forwards (+1 % length)
        engine->ExecuteClientCmd("mom_saveloc_nav_next");
        break;
    case 4://cycle backwards through savelocs
        engine->ExecuteClientCmd("mom_saveloc_nav_prev");
        break;
    case 5:// Go to the first saveloc
        engine->ExecuteClientCmd("mom_saveloc_nav_first");
        break;
    case 6:// Go to the last saveloc
        engine->ExecuteClientCmd("mom_saveloc_nav_last");
        break;
    case 7://remove current saveloc
        engine->ExecuteClientCmd("mom_saveloc_remove_current");
        break;
    case 8://remove every saveloc
        engine->ExecuteClientCmd("mom_saveloc_remove_all");
        break;
    case 0://They closed the menu
        engine->ExecuteClientCmd("mom_saveloc_close");
        break;
    default:
        C_BasePlayer* cPlayer = C_BasePlayer::GetLocalPlayer();
        if (cPlayer)
        {
            cPlayer->EmitSound("Momentum.UIMissingMenuSelection");
        }
        break;
    }
}

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
            KeyValues* pKv = new KeyValues("Saveloc Menu");
            pKv->AddSubKey(new KeyValues("#MOM_Menu_SaveCurLoc"));
            pKv->AddSubKey(new KeyValues("#MOM_Menu_ToCurrentSL"));
            pKv->AddSubKey(new KeyValues("#MOM_Menu_ToNextSL"));
            pKv->AddSubKey(new KeyValues("#MOM_Menu_ToPrevSL"));
            pKv->AddSubKey(new KeyValues("#MOM_Menu_ToFirstSL"));
            pKv->AddSubKey(new KeyValues("#MOM_Menu_ToLastSL"));
            pKv->AddSubKey(new KeyValues("#MOM_Menu_RemoveCurrentSL"));
            pKv->AddSubKey(new KeyValues("#MOM_Menu_RemoveEverySL"));
            savelocMenu->ShowMenu(pKv, SelectMenuItem, OnClose);
            pKv->deleteThis();
        }
    }
}