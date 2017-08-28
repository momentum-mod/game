#ifdef _WIN32
#include <windows.h>
#endif

#include "client_events.h"

#include "filesystem.h"
#include "mom_event_listener.h"
#include "mom_run_poster.h"
#include "movevars_shared.h"
#include "util/mom_util.h"
#include "momentum/ui/IMessageboxPanel.h"


#include "tier0/memdbgon.h"

extern IFileSystem *filesystem;

void CMOMClientEvents::PostInit()
{
    g_MOMEventListener->Init(); // Hook into game events
    g_MOMRunPoster->Init();     // Get ready to post runs...

    // enable console by default
    ConVarRef con_enable("con_enable");
    con_enable.SetValue(true);

    if (SteamAPI_IsSteamRunning())
    {
        g_pMomentumUtil->GetRemoteRepoModVersion();
    }

    // Mount CSS content even if it's on a different drive than SDK
    if (steamapicontext && steamapicontext->SteamApps())
    {
        char installPath[MAX_PATH];
        steamapicontext->SteamApps()->GetAppInstallDir(240, installPath, MAX_PATH);

        char pathCStrike[MAX_PATH];
        V_ComposeFileName(installPath, "cstrike", pathCStrike, sizeof(pathCStrike));
        filesystem->AddSearchPath(pathCStrike, "GAME");

        char pathPak[MAX_PATH];
        V_ComposeFileName(pathCStrike, "cstrike_pak.vpk", pathPak, sizeof(pathPak));
        filesystem->AddSearchPath(pathPak, "GAME");

        char downloadPath[MAX_PATH];
        V_ComposeFileName(pathCStrike, "download", downloadPath, sizeof(downloadPath));
        filesystem->AddSearchPath(downloadPath, "GAME");
        filesystem->AddSearchPath(downloadPath, "download");

#ifdef _DEBUG
        filesystem->PrintSearchPaths();
#endif

        // INTERESTING BUG, VOLVO!
        // Apparently, mounting sounds from any directory that isn't included in gameinfo.txt is impossible
        // to play without restarting the entire sound system. So we do that here.
        // If we had engine code...
        ConCommand *snd_restart = dynamic_cast<ConCommand*>(g_pCVar->FindCommand("snd_restart"));
        if (snd_restart)
        {
            char const *argv[1] = {"snd_restart"};
            CCommand cmd(1, argv);
            snd_restart->Dispatch(cmd);
        }
    }

    MountAdditionalContent();

    // Version warning
    // MOM_TODO: Change this once we hit Alpha/Beta
    // MOM_CURRENT_VERSION
    messageboxpanel->CreateMessageboxVarRef("#MOM_StartupMsg_Prealpha_Title", "#MOM_StartupMsg_Prealpha", "mom_toggle_versionwarn", "#MOM_IUnderstand");

    // So: If cl_software_cursor is 1 on game init, it won't use the software cursor for some reason, so we have to trick the engine 
    // so it obeys
    // If you find the reason, let @Rubén know (Probably the callback function is not being called on init, right?)
    ConVarRef cVarCursor("cl_software_cursor");
    if (cVarCursor.GetBool())
    {
        cVarCursor.SetValue(0);
        cVarCursor.SetValue(1);
    }
    
    if (!steamapicontext || !steamapicontext->SteamHTTP() || !steamapicontext->SteamUtils())
    {
        messageboxpanel->CreateMessagebox("#MOM_StartupMsg_NoSteamApiContext_Title", "#MOM_StartupMsg_NoSteamApiContext", "#MOM_IUnderstand");
    }
}

void CMOMClientEvents::MountAdditionalContent()
{
    // From the Valve SDK wiki
    KeyValues *pMainFile = new KeyValues("gameinfo.txt");
    bool bLoad = false;
#ifndef _WINDOWS
    // case sensitivity
    bLoad = pMainFile->LoadFromFile(filesystem, "GameInfo.txt", "MOD");
#endif
    if (!bLoad)
        bLoad = pMainFile->LoadFromFile(filesystem, "gameinfo.txt", "MOD");

    if (pMainFile && bLoad)
    {
        KeyValues *pFileSystemInfo = pMainFile->FindKey("FileSystem");
        if (pFileSystemInfo)
        {
            for (KeyValues *pKey = pFileSystemInfo->GetFirstSubKey(); pKey; pKey = pKey->GetNextKey())
            {
                if (Q_strcmp(pKey->GetName(), "AdditionalContentId") == 0)
                {
                    int appid = abs(pKey->GetInt());
                    if (appid)
                        if (filesystem->MountSteamContent(-appid) != FILESYSTEM_MOUNT_OK)
                            Warning("Unable to mount extra content with appId: %i\n", appid);
                }
            }
        }
    }
    pMainFile->deleteThis();
}

CMOMClientEvents g_MOMClientEvents("CMOMClientEvents");
