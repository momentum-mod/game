#include "cbase.h"
#include "hud_menu_static.h"
#include "mom_player_shared.h"

#include "tier0/memdbgon.h"

static void SelectMenuItem(int menu_item)
{
    switch (menu_item)
    {
    case 1:  //create a checkpoint
        engine->ExecuteClientCmd("mom_ruler_first");
        break;
    case 2:  // Select second point position
        engine->ExecuteClientCmd("mom_ruler_second");
        break;
    case 3:  // Measure distance between points
        engine->ExecuteClientCmd("mom_ruler_measure");
        break;
    case 4:  // Close the ruler
        engine->ExecuteClientCmd("mom_ruler_close");
        break;
    default:
        C_MomentumPlayer* cPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
        if (cPlayer != nullptr)
        {
            // How...
            cPlayer->EmitSound("Momentum.UIMissingMenuSelection");
        }
        break;
    }
}

CON_COMMAND(showRuler, "Opens the ruler tool.\n")
{
    CHudMenuStatic *rulerMenu = GET_HUDELEMENT(CHudMenuStatic);
    if (rulerMenu)
    {
        if (rulerMenu->IsMenuDisplayed())
        {
            rulerMenu->HideMenu();//MOM_TODO: if another menu is open this will close it!
            engine->ExecuteClientCmd("mom_ruler_close");
        }
        else
        {
            KeyValues* pKv = new KeyValues("Ruler Menu");
            pKv->AddSubKey(new KeyValues("#MOM_Ruler_FirstPoint"));
            pKv->AddSubKey(new KeyValues("#MOM_Ruler_SecondPoint"));
            pKv->AddSubKey(new KeyValues("#MOM_Ruler_Measure"));
            rulerMenu->ShowMenu(pKv, SelectMenuItem);
            pKv->deleteThis();
        }
    }
}