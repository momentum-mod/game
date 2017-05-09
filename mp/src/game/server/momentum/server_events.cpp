#include "cbase.h"
#include "server_events.h"
#include "mom_shareddefs.h"
#include "tier0/memdbgon.h"

zed_net_socket_t CMOMServerEvents::socket;
zed_net_address_t CMOMServerEvents::address;
char CMOMServerEvents::data[256];
CUtlVector<ghostNetFrame> CMOMServerEvents::ghostPlayers;

struct MyThreadParams_t
{
    CMomentumPlayer *pPlayer;
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
    m_ghostClientConnected = runGhostClient();

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
    m_ghostClientConnected = !exitGhostClient(); //set ghost client connection to false when we disconnect
}
void CMOMServerEvents::FrameUpdatePreEntityThink()
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetListenServerHost());
    if (isGhostClientConnected() && pPlayer)
    {
        if (ghostPlayers.Size() > 0)
            Msg("Players in ghost server: %i. Name of #0: %s,\n", ghostPlayers.Size(), ghostPlayers[0].PlayerName);

        MyThreadParams_t* vars = new MyThreadParams_t;
        vars->pPlayer = pPlayer;

        ThreadHandle_t thread = CreateSimpleThread(CMOMServerEvents::sendAndRecieveData, vars);
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
    ghostPlayers.RemoveAll();
    return true;
}
//Threaded function
unsigned CMOMServerEvents::sendAndRecieveData(void *params)
{
    MyThreadParams_t* vars = (MyThreadParams_t*)params; // always use a struct!

    //Send an identifier to the server that we're about to send a new frame. 
    //When we get an ACK from the server, we send the frame.
    int newFrameIdentifier = MOM_C_SENDING_NEWFRAME; //Client sending new data to server 
    zed_net_tcp_socket_send(&socket, &newFrameIdentifier, sizeof(newFrameIdentifier)); //SYN

    int data;
    int bytes_read = zed_net_tcp_socket_receive(&socket, &data, sizeof(data));

    if (bytes_read && data == MOM_C_RECIEVING_NEWFRAME) //SYN-ACK , Server acknowledges new frame is coming
    {
        ghostNetFrame newFrame(vars->pPlayer->EyeAngles(),
            vars->pPlayer->GetAbsOrigin(),
            vars->pPlayer->GetViewOffset(),
            vars->pPlayer->m_nButtons,
            vars->pPlayer->GetPlayerName());

        zed_net_tcp_socket_send(&socket, &newFrame, sizeof(newFrame)); //ACK
    }

    newFrameIdentifier = MOM_S_RECIEVING_NEWFRAME; //Client ready to recieve data from server 
    zed_net_tcp_socket_send(&socket, &newFrameIdentifier, sizeof(newFrameIdentifier)); //SYN

    bytes_read = zed_net_tcp_socket_receive(&socket, &data, sizeof(data));

    if (bytes_read && data == MOM_S_SENDING_NEWFRAME) //SYN-ACK
    {
        //The server then sends number of players to the client
        newFrameIdentifier = MOM_S_RECIEVING_NUMPLAYERS; //Client ready to recieve data from server 
        zed_net_tcp_socket_send(&socket, &newFrameIdentifier, sizeof(newFrameIdentifier)); //SYN

        bytes_read = zed_net_tcp_socket_receive(&socket, &data, sizeof(data));

        if (bytes_read && data == MOM_S_SENDING_NUMPLAYERS)
        {
            int playerNum = 0;
            bytes_read = zed_net_tcp_socket_receive(&socket, &playerNum, sizeof(playerNum));

            //TODO: Unseralize data
            //void* ghostData;
            //zed_net_tcp_socket_receive(&socket, &data, playerNum * sizeof(ghostNetFrame));
        }

    }
    delete vars;
    return 0;
}
CMOMServerEvents g_MOMServerEvents("CMOMServerEvents");