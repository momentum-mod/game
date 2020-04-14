#include "cbase.h"

#include "client_events.h"

#include "filesystem.h"
#include "MessageboxPanel.h"
#include "fmtstr.h"
#include "steam/steam_api.h"
#include "util/mom_util.h"

#include "icommandline.h"

#include "tier0/memdbgon.h"

extern IFileSystem *filesystem;

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