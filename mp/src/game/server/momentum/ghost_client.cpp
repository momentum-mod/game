#include "cbase.h"
#include "ghost_client.h"
#include "util/mom_util.h"
#include "tier0/memdbgon.h"
#include "mom_replay_entity.h"

ConVar mm_updaterate("mom_ghost_online_updaterate", "20", 
    FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE, 
    "Number of updates per second to and from the ghost server.\n", true, 1.0f, true, 50.0f);

ConVar mm_timeOutDuration("mom_ghost_online_timeout_duration", "10",
    FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE,
    "Seconds to wait when timimg out from a ghost server.\n", true, 5.0f, true, 30.0f);

ghostNetFrame_t CMomentumGhostClient::prevFrame;
zed_net_socket_t CMomentumGhostClient::m_socket;
zed_net_address_t CMomentumGhostClient::m_address;

char CMomentumGhostClient::data[256];
bool CMomentumGhostClient::m_ghostClientConnected = false;
bool CMomentumGhostClient::m_bRanThread = false;
const char* CMomentumGhostClient::m_host = "127.0.0.1";
unsigned short CMomentumGhostClient::m_port = 9000;

CMomentumPlayer* CMomentumGhostClient::m_pPlayer;
uint64 CMomentumGhostClient::m_SteamID;
CUtlVector<ghostNetFrame_t> CMomentumGhostClient::ghostPlayers;
CThreadMutex CMomentumGhostClient::m_mtxGhostPlayers;
CThreadMutex CMomentumGhostClient::m_mtxpPlayer;

ThreadHandle_t netIOThread;
void CMomentumGhostClient::LevelInitPostEntity()
{
    if (initGhostClient()) //init ghost client
    {
        m_ghostClientConnected = connectToGhostServer(m_host, m_port);
    }
    if (!m_ghostClientConnected)
    {
        exitGhostClient(); 
    }
}
void CMomentumGhostClient::LevelShutdownPreEntity()
{
    exitGhostClient(); //set ghost client connection to false when we disconnect
    m_ghostClientConnected = false;
}
void CMomentumGhostClient::FrameUpdatePostEntityThink()
{
    m_pPlayer = ToCMOMPlayer(UTIL_GetListenServerHost());

    // Run the thread that recieves and sends ghost data IFF we're connected to the server, AND it hasn't run before 
    if (!m_bRanThread && isGhostClientConnected() && m_pPlayer && steamapicontext)
    {
        MyThreadParams_t vars; //bogus params containing NOTHING hahAHAHAhaHHa
        //vars->pPlayer = m_pPlayer;
        m_SteamID = steamapicontext->SteamUser()->GetSteamID().ConvertToUint64();
        // Create a new thread to handle network I/O
        ConColorMsg(Color(0, 100, 255, 255), "Running thread\n");
        netIOThread = CreateSimpleThread(CMomentumGhostClient::sendAndRecieveData, &vars);
        ThreadDetach(netIOThread);
        m_bRanThread = true;
    }

    // DO stuff with that data
    if (isGhostClientConnected() && m_pPlayer)
    {
        m_mtxGhostPlayers.Lock();
        if (ghostPlayers.Size() > 0)
        {
            //MOM_TODO: Create ghost entity, and move them around. 
            //Msg("Players in server: %i", ghostPlayers.Size());
            for (int i = 0; i < ghostPlayers.Size(); i++)
            {
                Msg("Position of player #%i: %f, %f, %f\n", i, ghostPlayers[i].Position.x, ghostPlayers[i].Position.y, ghostPlayers[i].Position.z);
            }
        }
        m_mtxGhostPlayers.Unlock();
        
    }
}
bool CMomentumGhostClient::initGhostClient()
{
    m_socket = zed_net_socket_t();
    m_address = zed_net_address_t();
    if (zed_net_init() == 0)
    {
        int success = zed_net_tcp_socket_open(&m_socket, 0, 0, 0);
        return success == 0 ? true : false;
    }
    Warning("Could not init network! Error: %s\n", zed_net_get_error());
    return false;
}
// Returns true if we successfully exited
bool CMomentumGhostClient::exitGhostClient()
{
    int data = MOM_SIGNOFF;
    if (zed_net_tcp_socket_send(&m_socket, &data, sizeof(data)) < 0)
    {
        Warning("Could not send signoff packet! Error: %s\n", zed_net_get_error());
        return false;
    }
    int bytes_read = zed_net_tcp_socket_receive(&m_socket, &data, sizeof(data));
    if (bytes_read && data == MOM_SIGNOFF)
    {
        zed_net_socket_close(&m_socket);
        zed_net_shutdown();
        ConColorMsg(Color(255, 255, 0, 255), "Sent signoff packet, exiting ghost client...\n");
        m_mtxGhostPlayers.Lock();
        ghostPlayers.RemoveAll();
        m_mtxGhostPlayers.Unlock();
        return true;
    }
    Warning("Did not recieve ACK from server. Server will have to wait for us to time out...\n");
    return false;
}
bool CMomentumGhostClient::connectToGhostServer(const char* host, unsigned short port)
{
    if (zed_net_get_address(&m_address, host, port) != 0)
    {
        Warning("Error: %s\n", zed_net_get_error());

        zed_net_socket_close(&m_socket);
        zed_net_shutdown();
        return false;
    }

    if (zed_net_tcp_connect(&m_socket, m_address))
    {
        Warning("Failed to connect to %s:%d\n", host, port);
        return false;
    }
    ConColorMsg(Color(255, 255, 0, 255), "Connected to %s:%d\n", host, port);

    int data = MOM_SIGNON;
    zed_net_tcp_socket_send(&m_socket, &data, sizeof(data));
    ConColorMsg(Color(255, 255, 0, 255), "Sending signon packet...\n");
    char mapName[64];
    int bytes_read = zed_net_tcp_socket_receive(&m_socket, &mapName, sizeof(mapName));

    if (bytes_read) //Success!
    {
        if (Q_strcmp(mapName, gpGlobals->mapname.ToCStr()) == 0)
        {
            ConColorMsg(Color(255, 255, 0, 255), "Recieved ACK from server, we are on the same map: %s\n", mapName);
            m_bRanThread = false; //reset so we can run the connection thread again
            return true;
        }
        DevWarning("Recieved ACK from server, but could not syncronize maps! \
                               Server map: %s. Client map: %s. Disconnecting...\n", mapName, gpGlobals->mapname.ToCStr());
        return false;
    }
    DevWarning("Server did not ACK, we are not connected!\n");
    return false;

}
//Threaded function
unsigned CMomentumGhostClient::sendAndRecieveData(void *params)
{
    //Don't need a mutex for m_ghostClientConnected because if it ever changes from when the thread starts, the thread should exit anyways
    while (m_ghostClientConnected)
    {
        int tickSleep = 1000 * (float(1) / mm_updaterate.GetFloat());
        ThreadSleep(tickSleep); //sleep   
        //Send an identifier to the server that we're about to send a new frame. 
        //When we get an ACK from the server, we send the frame.
        int newFrameIdentifier = MOM_C_SENDING_NEWFRAME; //Client sending new data to server 
        zed_net_tcp_socket_send(&m_socket, &newFrameIdentifier, sizeof(newFrameIdentifier)); //SYN

        int data;
        int tick1 = gpGlobals->tickcount;
        int bytes_read = zed_net_tcp_socket_receive(&m_socket, &data, sizeof(data));
        while (bytes_read != 4)
        {
            int tick2 = gpGlobals->tickcount;
            float deltaT = float(tick2 - tick1) * gpGlobals->interval_per_tick;
            // Can't use ThreadSleep here for some reason since GabeN called me up and said "OH MY GOD WOULD YOU JUST FUCK OFF" when I tried to
            // ... um, I mean, ... It messes up gpGlobals->tickcount
            // So this code checks if deltaT is an integer, effectively only printing every 1 second.
            
            //MOM_TODO: Fix this (it prints... a lot..)
            if (deltaT == round(deltaT))
            {
                if (!m_ghostClientConnected) break; //prevents hang on hard quit
                Warning("Lost connection! Waiting to time out... %f\n", deltaT);
            }
            
            if (deltaT > mm_timeOutDuration.GetInt())
            {
                Warning("Lost connection to ghost server.\n");
                zed_net_socket_close(&m_socket);
                zed_net_shutdown();
                m_ghostClientConnected = false;
                return 1;
            }
        }
        m_mtxpPlayer.Lock();

        if (bytes_read && data == MOM_C_RECIEVING_NEWFRAME && m_pPlayer) //SYN-ACK , Server acknowledges new frame is coming
        {
            /*
            ghostAppearance_t newProps(m_pPlayer->GetModelName().ToCStr(),
                m_pPlayer->GetBodygroup(1),
                m_pPlayer->ColorAsRGBA(),
                m_pPlayer->TrailColorAsRGBA());
            */

            ghostNetFrame_t newFrame(m_pPlayer->EyeAngles(),
                m_pPlayer->GetAbsOrigin(),
                m_pPlayer->GetViewOffset(),
                m_pPlayer->m_nButtons,
                m_SteamID,
                m_pPlayer->GetPlayerName());

            zed_net_tcp_socket_send(&m_socket, &newFrame, sizeof(newFrame)); //ACK
        }
        m_mtxpPlayer.Unlock();

        newFrameIdentifier = MOM_S_RECIEVING_NEWFRAME; //Client ready to recieve data from server 
        zed_net_tcp_socket_send(&m_socket, &newFrameIdentifier, sizeof(newFrameIdentifier)); //SYN

        bytes_read = zed_net_tcp_socket_receive(&m_socket, &data, sizeof(data));

        if (bytes_read && data == MOM_S_SENDING_NEWFRAME) //SYN-ACK
        {
            //The server then sends number of players to the client
            int playerNum = 0;
            bytes_read = zed_net_tcp_socket_receive(&m_socket, &playerNum, sizeof(playerNum));

            m_mtxGhostPlayers.Lock();
            ghostNetFrame_t newFrame;
            for (int i = 0; i < playerNum; i++)
            {
                zed_net_tcp_socket_receive(&m_socket, &newFrame, playerNum * sizeof(ghostNetFrame_t));
                if (ghostPlayers.Size() == 0) //No players registered on client, we need to add new player 
                {
                    ghostPlayers.AddToTail(newFrame);
                }
                else
                {
                    uint64 steamID = ghostPlayers[ghostPlayers.Find(prevFrame)].SteamID64;
                    if (steamID != newFrame.SteamID64) //couldn't find the player already in the playerlist
                    {
                        ghostPlayers.AddToTail(newFrame);
                    }
                    else
                    {
                        ghostPlayers[i] = newFrame;
                    }
                }
            }
            prevFrame = newFrame;
            m_mtxGhostPlayers.Unlock();
        }
        if (bytes_read && data == MOM_S_SENDING_NEWMAP)
        {
            char newMapName[64];
            zed_net_tcp_socket_receive(&m_socket, newMapName, sizeof(newMapName));
            if (Q_strcmp(gpGlobals->mapname.ToCStr(), newMapName) != 0) //The new map is different from the one we are currently playing on
            {
                engine->ClientCommand(m_pPlayer->edict(), "map %s", newMapName);
                return 0; //exit the thread
            }
        }
    }
    return 0;
}
static CMomentumGhostClient s_MOMGhostClient;
CMomentumGhostClient *g_pMomentumGhostClient = &s_MOMGhostClient;