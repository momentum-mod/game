#include "cbase.h"

#include "client_events.h"

#include "filesystem.h"
#include "IMessageboxPanel.h"
#include "fmtstr.h"
#include "steam/steam_api.h"

#include "icommandline.h"

#include "tier0/memdbgon.h"

extern IFileSystem *filesystem;

inline void UnloadConVarOrCommand(const char *pName)
{
    const auto pCmd = g_pCVar->FindCommandBase(pName);
    if (pCmd)
        g_pCVar->UnregisterConCommand(pCmd);
}

bool CMOMClientEvents::Init()
{
    // Mount CSS content even if it's on a different drive than this game
    if (SteamApps())
    {
        char installPath[MAX_PATH];
        uint32 folderLen = SteamApps()->GetAppInstallDir(240, installPath, MAX_PATH);
        if (folderLen)
        {
            filesystem->AddSearchPath(CFmtStr("%s/cstrike", installPath), "GAME");
            filesystem->AddSearchPath(CFmtStr("%s/cstrike/cstrike_pak.vpk", installPath), "GAME");
            filesystem->AddSearchPath(CFmtStr("%s/cstrike/download", installPath), "GAME");
            filesystem->AddSearchPath(CFmtStr("%s/cstrike/download", installPath), "download");

            if (developer.GetInt())
                filesystem->PrintSearchPaths();
        }
    }

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
    messageboxpanel->CreateMessageboxVarRef("#MOM_StartupMsg_Alpha_Title", "#MOM_StartupMsg_Alpha",
                                            "mom_toggle_versionwarn", "#MOM_IUnderstand");
    
    if (!SteamAPI_IsSteamRunning() || !SteamHTTP())
    {
        vgui::Panel *pPanel = messageboxpanel->CreateMessagebox("#MOM_StartupMsg_NoSteamApiContext_Title",
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