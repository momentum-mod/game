#include "cbase.h"
#include "hud_menu_static.h"
#include "filesystem.h"

#include "tier0/memdbgon.h"

static KeyValues *pSavelocKV = nullptr;

static void SelectMenuItem(int menu_item)
{
    if (!pSavelocKV)
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
    KeyValues *pButtonKV = pSavelocKV->FindKey(strMenuItem);
    if (pButtonKV) // menu item is defined in KVs
    {
        engine->ExecuteClientCmd(pButtonKV->FindKey("command")->GetString());
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
            if (pSavelocKV)
                pSavelocKV->deleteThis();

            pSavelocKV = new KeyValues("Saveloc Menu");
            pSavelocKV->LoadFromFile(g_pFullFileSystem, "cfg/savelocmenu.vdf", "MOD");

            KeyValues *pKv = new KeyValues(pSavelocKV->GetString());
            char strMenuItem[4];

            // keep 9 max (0 for cancel & >10 isn't reachable)
            // also retrieves by number, not by order in file, & ignores the rest
            for (int iCtr = 1; iCtr <= 9; iCtr++)
            {
                Q_snprintf(strMenuItem, sizeof(strMenuItem), "%i", iCtr);
                KeyValues *subKV = pSavelocKV->FindKey(strMenuItem);

                if (!subKV) // no more defined
                    break;

                pKv->AddSubKey(new KeyValues(subKV->FindKey("label")->GetString()));
            }

            savelocMenu->ShowMenu(pKv, SelectMenuItem, OnClose);
            pKv->deleteThis();
        }
    }
}