#ifdef _WIN32
#include <windows.h>
#endif

#include "client_events.h"

#include "filesystem.h"
#include "mom_event_listener.h"
#include "mom_run_poster.h"
#include "movevars_shared.h"
#include "util/mom_util.h"

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
        mom_UTIL->GetRemoteRepoModVersion();
    }

// mount CSS content even if it's on a different drive than SDK
#ifdef _WIN32
    HKEY hKey;
    if (VCRHook_RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                             "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App 240", 0, KEY_READ,
                             &hKey) == ERROR_SUCCESS)
    {
        char installPath[MAX_PATH];
        DWORD len = sizeof(installPath);
        if (VCRHook_RegQueryValueEx(hKey, "InstallLocation", NULL, NULL, (LPBYTE)installPath, &len) == ERROR_SUCCESS)
        {
            char path[MAX_PATH];
            Q_strncpy(path, installPath, sizeof(path));

            Q_strncat(path, "\\cstrike", sizeof(path));
            filesystem->AddSearchPath(path, "GAME");

            Q_strncat(path, "\\download", sizeof(path));
            filesystem->AddSearchPath(path, "GAME");

            Q_strncpy(path, installPath, sizeof(path));
            Q_strncat(path, "\\cstrike\\cstrike_pak.vpk", sizeof(path));
            filesystem->AddSearchPath(path, "GAME");

            filesystem->PrintSearchPaths();
        }

        VCRHook_RegCloseKey(hKey);
    }
#endif

    MountAdditionalContent();
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
