#include "cbase.h"

#include "client_events.h"

#include "filesystem.h"
#include "mom_event_listener.h"
#include "mom_run_poster.h"
#include "momentum/ui/IMessageboxPanel.h"
#include "fmtstr.h"

#include "tier0/memdbgon.h"

extern IFileSystem *filesystem;

bool CMOMClientEvents::Init()
{
    // Mount CSS content even if it's on a different drive than this game
    if (steamapicontext && steamapicontext->SteamApps())
    {
        char installPath[MAX_PATH];
        uint32 folderLen = steamapicontext->SteamApps()->GetAppInstallDir(240, installPath, MAX_PATH);
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

    return true;
}

void CMOMClientEvents::PostInit()
{
    g_MOMEventListener->Init(); // Hook into game events
    //g_MOMRunPoster->Init();     // Get ready to post runs...

    // enable console by default
    ConVarRef con_enable("con_enable");
    con_enable.SetValue(true);

    // Version warning
    // MOM_TODO: Change this once we hit Alpha/Beta
    // MOM_CURRENT_VERSION
    messageboxpanel->CreateMessageboxVarRef("#MOM_StartupMsg_Prealpha_Title", "#MOM_StartupMsg_Prealpha", "mom_toggle_versionwarn", "#MOM_IUnderstand");
    
    if (!steamapicontext || !steamapicontext->SteamHTTP() || !steamapicontext->SteamUtils())
    {
        messageboxpanel->CreateMessagebox("#MOM_StartupMsg_NoSteamApiContext_Title", "#MOM_StartupMsg_NoSteamApiContext", "#MOM_IUnderstand");
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
