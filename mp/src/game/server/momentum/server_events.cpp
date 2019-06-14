#include "cbase.h"
#include "filesystem.h"
#include "server_events.h"
#include "tickset.h"
#include "icommandline.h"

#include "tier0/memdbgon.h"

CMomServerEvents::CMomServerEvents(): CAutoGameSystem("CMomServerEvents")
{
}

inline void UnloadConVarOrCommand(const char *pName)
{
    const auto pCmd = g_pCVar->FindCommandBase(pName);
    if (pCmd)
        g_pCVar->UnregisterConCommand(pCmd);
}

bool CMomServerEvents::Init()
{
    if (!CommandLine()->FindParm("-mapping"))
    {
        // Check plugins
        FileFindHandle_t handle;
        const auto pFound = g_pFullFileSystem->FindFirstEx("addons/*.vdf", "GAME", &handle);
        if (pFound)
            Error("Boot the game with -mapping to be able to use plugins!");

        g_pFullFileSystem->FindClose(handle);

        UnloadConVarOrCommand("plugin_load");

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

void CMomServerEvents::PostInit()
{
    TickSet::TickInit();
    MountAdditionalContent();
}

void CMomServerEvents::LevelShutdownPostEntity()
{
    ConVarRef fullbright("mat_fullbright");
    // Shut off fullbright if the map enabled it
    if (fullbright.IsValid() && fullbright.GetBool())
        fullbright.SetValue(0);
}

void CMomServerEvents::OnGameOverlay(GameOverlayActivated_t* pParam)
{
    engine->ServerCommand("unpause\n");
}

void CMomServerEvents::MountAdditionalContent()
{
    // From the Valve SDK wiki
    KeyValues *pMainFile = new KeyValues("gameinfo.txt");
    bool bLoad = false;
#ifndef _WIN32
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

CMomServerEvents g_MOMServerEvents;