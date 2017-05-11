#include "cbase.h"
#include "ghost_client.h"
#include "util/mom_util.h"
#include "tier0/memdbgon.h"
#include "mom_replay_entity.h"

ConVar mm_updaterate("mom_ghost_online_updaterate", "66", FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE, "Number of updates per second to and from the ghost server.\n", true, 20.0f, true, 100.0f);

ghostNetFrame CMomentumGhostClient::prevFrame;
zed_net_socket_t CMomentumGhostClient::socket;
zed_net_address_t CMomentumGhostClient::address;
char CMomentumGhostClient::data[256];
bool CMomentumGhostClient::m_ghostClientConnected = false;
bool CMomentumGhostClient::m_bRanThread = false;
CMomentumPlayer* CMomentumGhostClient::m_pPlayer;
uint64 CMomentumGhostClient::m_SteamID;
CUtlVector<ghostNetFrame> CMomentumGhostClient::ghostPlayers;
CThreadMutex CMomentumGhostClient::m_mtxGhostPlayers;
CThreadMutex CMomentumGhostClient::m_mtxpPlayer;
CThreadMutex CMomentumGhostClient::m_mtxbClientConnect;

ThreadHandle_t netIOThread;
void CMomentumGhostClient::LevelInitPostEntity()
{
    m_ghostClientConnected = runGhostClient();
}
void CMomentumGhostClient::LevelShutdownPreEntity()
{
    m_ghostClientConnected = !exitGhostClient(); //set ghost client connection to false when we disconnect
    //ThreadJoin(netIOThread);
}
void CMomentumGhostClient::FrameUpdatePostEntityThink()
{
    m_pPlayer = ToCMOMPlayer(UTIL_GetListenServerHost());

    // Run the thread that recieves and sends ghost data IFF it hasn't run before 
    if (!m_bRanThread && m_pPlayer && steamapicontext)
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
            Msg("Position of player #%i: %f, %f, %f\n", 0, ghostPlayers[0].Position.x, ghostPlayers[0].Position.y, ghostPlayers[0].Position.z);
            //MOM_TODO: Create ghost entity, and move them around. 
            //Msg("Players in server: %i", ghostPlayers.Size());
            //for (int i = 0; i < ghostPlayers.Size(); i++)
            //{
                //Msg("STEAMID of idx %i: %llu\n", i, ghostPlayers[i].SteamID64);
            //}
        }
        m_mtxGhostPlayers.Unlock();
        
    }
}
bool CMomentumGhostClient::runGhostClient()
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
    int bytes_read = zed_net_tcp_socket_receive(&socket, &data, sizeof(data));

    if (bytes_read && data == MOM_SIGNON)
    {
        ConColorMsg(Color(255, 255, 0, 255), "Recieved ACK from server...\n");
        m_bRanThread = false; //reset so we can run the connection thread again
        return true;
    }
    Warning("Server did not ACK, we are not connected!\n");
    return false;
}
// Returns true if we successfully exited
bool CMomentumGhostClient::exitGhostClient()
{
    int data = MOM_SIGNOFF;
    if (zed_net_tcp_socket_send(&socket, &data, sizeof(data)) < 0)
    {
        Warning("Could not send signoff packet! Error: %s\n", zed_net_get_error());
        return false;
    }

    zed_net_socket_close(&socket);
    zed_net_shutdown();
    ConColorMsg(Color(255, 255, 0, 255), "Sent signoff packet, exiting ghost client...\n");
    m_mtxGhostPlayers.Lock();
    ghostPlayers.RemoveAll();
    m_mtxGhostPlayers.Unlock();
    return true;
}

//Threaded function
unsigned CMomentumGhostClient::sendAndRecieveData(void *params)
{
    m_mtxbClientConnect.Lock();
    while (m_ghostClientConnected)
    {
        int tickSleep = 1000 * (1 / mm_updaterate.GetInt());
        ThreadSleep(tickSleep); //sleep for 10ms  
        //Send an identifier to the server that we're about to send a new frame. 
        //When we get an ACK from the server, we send the frame.
        int newFrameIdentifier = MOM_C_SENDING_NEWFRAME; //Client sending new data to server 
        zed_net_tcp_socket_send(&socket, &newFrameIdentifier, sizeof(newFrameIdentifier)); //SYN

        int data;
        int bytes_read = zed_net_tcp_socket_receive(&socket, &data, sizeof(data));

        m_mtxpPlayer.Lock();

        if (bytes_read && data == MOM_C_RECIEVING_NEWFRAME && m_pPlayer) //SYN-ACK , Server acknowledges new frame is coming
        {
            ghostNetFrame newFrame(m_pPlayer->EyeAngles(),
                m_pPlayer->GetAbsOrigin(),
                m_pPlayer->GetViewOffset(),
                m_pPlayer->m_nButtons,
                m_SteamID);
            zed_net_tcp_socket_send(&socket, &newFrame, sizeof(newFrame)); //ACK
        }
        m_mtxpPlayer.Unlock();

        newFrameIdentifier = MOM_S_RECIEVING_NEWFRAME; //Client ready to recieve data from server 
        zed_net_tcp_socket_send(&socket, &newFrameIdentifier, sizeof(newFrameIdentifier)); //SYN

        bytes_read = zed_net_tcp_socket_receive(&socket, &data, sizeof(data));

        if (bytes_read && data == MOM_S_SENDING_NEWFRAME) //SYN-ACK
        {
            //The server then sends number of players to the client
            int playerNum = 0;
            bytes_read = zed_net_tcp_socket_receive(&socket, &playerNum, sizeof(playerNum));

            m_mtxGhostPlayers.Lock();
            ghostNetFrame newFrame;
            for (int i = 0; i < playerNum; i++)
            {
                zed_net_tcp_socket_receive(&socket, &newFrame, playerNum * sizeof(ghostNetFrame));
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
    }
    m_mtxbClientConnect.Unlock();
    return 0;
}
static CMomentumGhostClient s_MOMGhostClient;
CMomentumGhostClient *g_pMomentumGhostClient = &s_MOMGhostClient;