#include "cbase.h"
#include "filesystem.h"
#include "server_events.h"
#include "tickset.h"
#include "mapzones.h"
#include "mom_timer.h"
#include "fmtstr.h"

#include "tier0/memdbgon.h"

CMOMServerEvents::CMOMServerEvents(const char* pName): CAutoGameSystem(pName), zones(nullptr)
{
}

void CMOMServerEvents::PostInit()
{
    TickSet::TickInit();
    MountAdditionalContent();

    // MOM_TODO: connect to site
    /*if (SteamAPI_IsSteamRunning())
    {

    }*/
}

void CMOMServerEvents::Shutdown()
{


}

void CMOMServerEvents::LevelInitPreEntity()
{
    const char *pMapName = gpGlobals->mapname.ToCStr();
    // (Re-)Load zones
    if (zones)
    {
        delete zones;
        zones = nullptr;
    }
    zones = new CMapZoneData(pMapName);
    zones->SpawnMapZones();
}


void CMOMServerEvents::LevelInitPostEntity()
{
    //disable point_servercommand
    ConVarRef pointcommand("sv_allow_point_command");
    pointcommand.SetValue("disallow");
}

void CMOMServerEvents::LevelShutdownPreEntity()
{
    // Unload zones
    if (zones)
    {
        delete zones;
        zones = nullptr;
    }
}

void CMOMServerEvents::LevelShutdownPostEntity()
{
    ConVarRef fullbright("mat_fullbright");
    // Shut off fullbright if the map enabled it
    if (fullbright.IsValid() && fullbright.GetBool())
        fullbright.SetValue(0);
}

void CMOMServerEvents::OnGameOverlay(GameOverlayActivated_t* pParam)
{
    engine->ServerCommand("unpause\n");
}

void CMOMServerEvents::MountAdditionalContent()
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

CMOMServerEvents g_MOMServerEvents("CMOMServerEvents");