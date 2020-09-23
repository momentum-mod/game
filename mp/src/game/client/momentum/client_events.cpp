#include "cbase.h"

#include "client_events.h"

#include "filesystem.h"
#include "MessageboxPanel.h"
#include "fmtstr.h"
#include "steam/steam_api.h"
#include "util/mom_util.h"

#include "icommandline.h"
#include "toolframework/ienginetool.h"
#include "client_factorylist.h"
#include "run/mom_run_safeguards.h"
#include "gameui/BaseMenuPanel.h"

#include "tier0/memdbgon.h"

#undef nullptr

inline void UnloadConVarOrCommand(const char *pName)
{
    const auto pCmd = g_pCVar->FindCommandBase(pName);
    if (pCmd)
        g_pCVar->UnregisterConCommand(pCmd);
}

static MAKE_TOGGLE_CONVAR(__game_quit_ok, "0", FCVAR_HIDDEN | FCVAR_CLIENTCMD_CAN_EXECUTE, "");

bool CanQuit(void *pUnused)
{
    NOTE_UNUSED(pUnused);

    if (g_pRunSafeguards->IsSafeguarded(RUN_SAFEGUARD_QUIT_GAME) && !__game_quit_ok.GetBool())
    {
        g_pMessageBox->CreateConfirmationBox(g_pBasePanel->GetMainMenu(), "#MOM_MB_Safeguard_Map_Quit_Game_Title", "#MOM_MB_Safeguard_Map_Quit_Game_Msg", new KeyValues("ConfirmQuit"), nullptr, "#GameUI2_Quit", "#GameUI_Cancel");

        return false;
    }

    g_pBasePanel->OnShutdownFromQuit();

    return true;
}

bool CMOMClientEvents::Init()
{
    // Mount CSS content even if it's on a different drive than this game
    MomUtil::MountGameFiles();

    if (!CommandLine()->FindParm("-mapping"))
    {
        // Unregister FCVAR_MAPPING convars
        auto pCvar = g_pCVar->GetCommands();
        while (pCvar)
        {
            const auto pNext = pCvar->GetNext();

            if (pCvar->IsFlagSet(FCVAR_MAPPING))
                g_pCVar->UnregisterConCommand(pCvar);

            pCvar = pNext;
        }
    }

    UnloadConVarOrCommand("retry");

    static ConCommand retry("retry", []()
    {
        engine->ExecuteClientCmd("reload");
    });

    factorylist_t list;
    FactoryList_Retrieve(list);

    const auto pTool = static_cast<IEngineTool*>(list.appSystemFactory(VENGINETOOL_INTERFACE_VERSION, NULL));

    if (!pTool)
        return false;

    pTool->InstallQuitHandler(nullptr, &CanQuit);

    return true;
}

void CMOMClientEvents::PostInit()
{
    // enable console by default
    ConVarRef con_enable("con_enable");
    con_enable.SetValue(true);

    // Version warning
    // MOM_TODO: Change this once we hit Beta
    // MOM_CURRENT_VERSION
    g_pMessageBox->CreateMessageboxVarRef("#MOM_StartupMsg_Alpha_Title", "#MOM_StartupMsg_Alpha",
                                            "mom_toggle_versionwarn", "#MOM_IUnderstand");
    
    if (!SteamAPI_IsSteamRunning() || !SteamHTTP())
    {
        vgui::Panel *pPanel = g_pMessageBox->CreateMessagebox("#MOM_StartupMsg_NoSteamApiContext_Title",
                                                        "#MOM_StartupMsg_NoSteamApiContext", "#MOM_IUnderstand");
        pPanel->MoveToFront();
        pPanel->RequestFocus();
    }
}

void CMOMClientEvents::LevelInitPreEntity()
{
    //Precache();
}

void CMOMClientEvents::Precache()
{
    // MOM_TODO: Precache anything here
}

CMOMClientEvents g_MOMClientEvents("CMOMClientEvents");