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
    case 0:
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
            KeyValues* pKv = new KeyValues("Ruler Menu");
            pKv->AddSubKey(new KeyValues("#MOM_Ruler_FirstPoint"));
            pKv->AddSubKey(new KeyValues("#MOM_Ruler_SecondPoint"));
            pKv->AddSubKey(new KeyValues("#MOM_Ruler_Measure"));
            rulerMenu->ShowMenu(pKv, SelectMenuItem, OnClose);
            pKv->deleteThis();
        }
    }
}