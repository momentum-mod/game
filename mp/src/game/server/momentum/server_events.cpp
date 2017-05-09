#include "cbase.h"
#include "server_events.h"
#include "mom_shareddefs.h"
#include "tier0/memdbgon.h"

zed_net_socket_t CMOMServerEvents::socket;
zed_net_address_t CMOMServerEvents::address;
char CMOMServerEvents::data[256];

struct MyThreadParams_t
{
    int number;
    char letter;
};

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
        gm.SetValue(MOMGM_SURF);
    }
    else if (!Q_strnicmp(pMapName, "bhop_", strlen("bhop_")))
    {
        DevLog("SETTING THE GAMEMODE!\n");
        gm.SetValue(MOMGM_BHOP);
    }
    else if (!Q_strnicmp(pMapName, "kz_", strlen("kz_")))
    {
        DevLog("SETTING THE GAMEMODE!\n");
        gm.SetValue(MOMGM_SCROLL);
    }
    else if (!Q_strcmp(pMapName, "background") || !Q_strcmp(pMapName, "credits"))
    {
        gm.SetValue(MOMGM_ALLOWED);
    }
    else
    {
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
    runGhostClient();

    // Reset zone editing
    g_MapzoneEdit.Reset();

    //disable point_servercommand
    ConVarRef servercommand("sv_allow_point_servercommand");
    servercommand.SetValue("0");
}

void CMOMServerEvents::LevelShutdownPreEntity()
{
    // Unload zones
    if (zones)
    {
        delete zones;
        zones = nullptr;
    }

    ConVarRef gm("mom_gamemode");
    gm.SetValue(gm.GetDefault());
}

void CMOMServerEvents::LevelShutdownPostEntity()
{
    ConVarRef fullbright("mat_fullbright");
    // Shut off fullbright if the map enabled it
    if (fullbright.IsValid() && fullbright.GetBool())
        fullbright.SetValue(0);
    exitGhostClient();
}
void CMOMServerEvents::FrameUpdatePreEntityThink()
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetListenServerHost());
    if (isGhostClientConnected() && pPlayer)
    {
        MyThreadParams_t* vars = new MyThreadParams_t;
        //vars->number = 5;

        ThreadHandle_t thread = CreateSimpleThread(CMOMServerEvents::recieveGhostData, vars);
        ThreadDetach(thread);
    }
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
    Msg("Recieved from ghost server: %s", data);
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

bool CMOMServerEvents::runGhostClient()
{
    socket = zed_net_socket_t();
    address = zed_net_address_t();
    zed_net_init();
    zed_net_tcp_socket_open(&socket, 0, 0, 0);

    if (zed_net_get_address(&address, host, port) != 0)
    {
        Warning("Error: %s\n", zed_net_get_error());

        zed_net_socket_close(&socket);
        zed_net_shutdown();
        return false;
    }

    if (zed_net_tcp_connect(&socket, address))
    {
        Warning("Failed to connect to %s:%d\n", host, port);
        return false;
    }
    ConColorMsg(Color(255, 255, 0, 255), "Connected to %s:%d\n", host, port);

    int data = MOM_SIGNON;
    zed_net_tcp_socket_send(&socket, &data, sizeof(data));
    ConColorMsg(Color(255, 255, 0, 255), "Sending signon packet...\n");
    return true;
}
bool CMOMServerEvents::exitGhostClient()
{
    int data = MOM_SIGNOFF;
    if (zed_net_tcp_socket_send(&socket, &data, sizeof(data)))
    {
        Warning("Could not send signoff packet!\n");
        return false;
    }

    zed_net_socket_close(&socket);
    zed_net_shutdown();
    ConColorMsg(Color(255, 255, 0, 255), "Sent signoff packet, exiting ghost client...\n");
    return true;
}
unsigned CMOMServerEvents::recieveGhostData(void *params)
{
    MyThreadParams_t* vars = (MyThreadParams_t*)params; // always use a struct!

    char buffer[256];
    int bytes_read = zed_net_tcp_socket_receive(&socket, buffer, sizeof(buffer));
    if (bytes_read)
    {
        Q_strcpy(data, buffer);
    }
    delete vars;
    return 0;
}
//Create the 
CMOMServerEvents g_MOMServerEvents("CMOMServerEvents");