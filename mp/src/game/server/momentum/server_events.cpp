#include "cbase.h"
#include "server_events.h"
#include "mom_shareddefs.h"
#include "tier0/memdbgon.h"


//This is only called when "map ____" is called, if the user uses changelevel then...
// \/(o_o)\/
void Momentum::GameInit()
{
    ConVarRef gm("mom_gamemode");
    ConVarRef map("host_map");
    const char *pMapName = map.GetString();
    // This will only happen if the user didn't use the map selector to start a map
    ConVarRef("sv_contact").SetValue("http://momentum-mod.org/contact");
    //set gamemode depending on map name
    //MOM_TODO: This needs to read map entity/momfile data and set accordingly

    if (!Q_strnicmp(pMapName, "surf_", strlen("surf_")))
    {
        DevLog("Setting game mode to surf (GM# %d)\n", MOMGM_SURF);
        gm.SetValue(MOMGM_SURF);
    }
    else if (!Q_strnicmp(pMapName, "bhop_", strlen("bhop_")))
    {
        DevLog("Setting game mode to bhop (GM# %d)\n", MOMGM_BHOP);
        gm.SetValue(MOMGM_BHOP);
    }
    else if (!Q_strnicmp(pMapName, "kz_", strlen("kz_")))
    {
        DevLog("Setting game mode to scroll (GM# %d)\n", MOMGM_SCROLL);
        gm.SetValue(MOMGM_SCROLL);
    }
    else if (!Q_strcmp(pMapName, "background") || !Q_strcmp(pMapName, "credits"))
    {
        gm.SetValue(MOMGM_ALLOWED);
    }
    else
    {
        DevLog("Setting game mode to unknown\n");
        gm.SetValue(MOMGM_UNKNOWN);
    }
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

void CMOMServerEvents::LevelInitPreEntity()
{
    const char *pMapName = gpGlobals->mapname.ToCStr();
    // (Re-)Load zones
    if (zones)
    {
        delete zones;
        zones = nullptr;
    }
    zones = new CMapzoneData(pMapName);
    zones->SpawnMapZones();
}


void CMOMServerEvents::LevelInitPostEntity()
{
    // Reset zone editing
    g_MapzoneEdit.Reset();

    //disable point_servercommand
    ConVarRef servercommand("sv_allow_point_servercommand");
    servercommand.SetValue(0);
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
void CMOMServerEvents::FrameUpdatePreEntityThink()
{
    g_MapzoneEdit.Update();

    if (!g_pMomentumTimer->GotCaughtCheating())
    {
        ConVarRef cheatsRef("sv_cheats");
        if (cheatsRef.GetBool())
        {
            g_pMomentumTimer->SetCheating(true);
            g_pMomentumTimer->Stop(false);
        }
    }
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