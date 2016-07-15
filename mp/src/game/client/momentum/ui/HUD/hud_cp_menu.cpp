#include "cbase.h"
#include "hud_menu_static.h"
#include "mom_player_shared.h"

#include "tier0/memdbgon.h"

static void SelectMenuItem(int menu_item)
{
    C_MomentumPlayer* cPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    switch (menu_item)
    {
    case 1://create a checkpoint
        engine->ExecuteClientCmd("mom_checkpoint_create");
        break;
    case 2://load previous checkpoint
        engine->ExecuteClientCmd("mom_checkpoint_prev");
        break;
    case 3://cycle through checkpoints forwards (+1 % length)
        engine->ExecuteClientCmd("mom_checkpoint_nav_next");
        break;
    case 4://cycle backwards through checkpoints
        engine->ExecuteClientCmd("mom_checkpoint_nav_prev");
        break;
    case 5://remove current checkpoint
        engine->ExecuteClientCmd("mom_checkpoint_remove_prev");
        break;
    case 6://remove every checkpoint
        engine->ExecuteClientCmd("mom_checkpoint_remove_all");
        break;
    case 0://They closed the menu
        engine->ExecuteClientCmd("mom_checkpoint_close");
        break;
    default:
        if (cPlayer != nullptr)
        {
            cPlayer->EmitSound("Momentum.UIMissingMenuSelection");
        }
        break;
    }
}

CON_COMMAND(showCPmenu, "Opens the Checkpoint Menu.\n")
{
    CHudMenuStatic *cpMenu = GET_HUDELEMENT(CHudMenuStatic);
    if (cpMenu)
    {
        if (cpMenu->IsMenuDisplayed())
        {
            cpMenu->HideMenu();//MOM_TODO: if another menu is open this will close it!
            engine->ExecuteClientCmd("mom_checkpoint_close");
        }
        else
        {
            KeyValues* pKv = new KeyValues("CP Menu");
            pKv->AddSubKey(new KeyValues("#MOM_Menu_CreateCP"));
            pKv->AddSubKey(new KeyValues("#MOM_Menu_ToPreviousCP"));
            pKv->AddSubKey(new KeyValues("#MOM_Menu_ToNextCP"));
            pKv->AddSubKey(new KeyValues("#MOM_Menu_ToLastCP"));
            pKv->AddSubKey(new KeyValues("#MOM_Menu_RemoveCurrentCP"));
            pKv->AddSubKey(new KeyValues("#MOM_Menu_RemoveEveryCP"));
            cpMenu->ShowMenu(pKv, SelectMenuItem);
            pKv->deleteThis();
        }
    }
}