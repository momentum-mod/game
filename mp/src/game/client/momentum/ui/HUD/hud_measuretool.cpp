#include "cbase.h"
#include "hud_menu_static.h"
#include "mom_player_shared.h"
#include "filesystem.h"

#include "tier0/memdbgon.h"

static KeyValues *pRulerKV = nullptr;

static void SelectMenuItem(int menu_item)
{
    if (!pRulerKV)
        return;

    if (menu_item == 0) // close button
    {
        engine->ExecuteClientCmd("mom_saveloc_close");
        return;
    }

    if (menu_item < 0 || menu_item > 9) // unknown
    {
        C_BasePlayer *cPlayer = C_BasePlayer::GetLocalPlayer();
        if (cPlayer)
        {
            cPlayer->EmitSound("Momentum.UIMissingMenuSelection");
        }
        return;
    }

    char strMenuItem[4];
    Q_snprintf(strMenuItem, sizeof(strMenuItem), "%i", menu_item);
    KeyValues *pButtonKV = pRulerKV->FindKey(strMenuItem);
    if (pButtonKV) // menu item is defined in KVs
    {
        engine->ExecuteClientCmd(pButtonKV->FindKey("command")->GetString());
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
            if (pRulerKV)
                pRulerKV->deleteThis();

            pRulerKV = new KeyValues("Ruler Menu");
            pRulerKV->LoadFromFile(g_pFullFileSystem, "cfg/rulermenu.vdf", "MOD");

            KeyValues *pKv = new KeyValues(pRulerKV->GetString());
            char strMenuItem[4];

            // keep 9 max (0 for cancel & >10 isn't reachable)
            // also retrieves by number, not by order in file, & ignores the rest
            for (int iCtr = 1; iCtr <= 9; iCtr++)
            {
                Q_snprintf(strMenuItem, sizeof(strMenuItem), "%i", iCtr);
                KeyValues *subKV = pRulerKV->FindKey(strMenuItem);

                if (!subKV) // no more defined
                    break;

                pKv->AddSubKey(new KeyValues(subKV->FindKey("label")->GetString()));
            }

            rulerMenu->ShowMenu(pKv, SelectMenuItem, OnClose);
            pKv->deleteThis();
        }
    }
}