#include "cbase.h"
#include "ghost_client.h"
#include "util/mom_util.h"
#include "tier0/memdbgon.h"

struct MyThreadParams_t
{
    CMomentumPlayer *pPlayer;
};
ghostNetFrame CMomentumGhostClient::prevFrame;
zed_net_socket_t CMomentumGhostClient::socket;
zed_net_address_t CMomentumGhostClient::address;
char CMomentumGhostClient::data[256];
CUtlVector<ghostNetFrame> CMomentumGhostClient::ghostPlayers;

void CMomentumGhostClient::LevelInitPostEntity()
{
    runGhostClient();
}
void CMomentumGhostClient::LevelShutdownPostEntity()
{
    m_ghostClientConnected = !exitGhostClient(); //set ghost client connection to false when we disconnect
}
void CMomentumGhostClient::FrameUpdatePreEntityThink()
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetListenServerHost());
    if (isGhostClientConnected() && pPlayer && steamapicontext) 
    {
        if (ghostPlayers.Size() > 0)
        {
            //Msg("Players in ghost server: %i. SteamID of idx #0: %llu\n", ghostPlayers.Size(), ghostPlayers[0].SteamID64);
            //Msg("Position of player: %f, %f, %f\n", ghostPlayers[0].Position.x, ghostPlayers[0].Position.y, ghostPlayers[0].Position.z);
        }
        MyThreadParams_t* vars = new MyThreadParams_t;
        vars->pPlayer = pPlayer;
        // Create a new thread to handle network I/O
        ThreadHandle_t thread = CreateSimpleThread(CMomentumGhostClient::sendAndRecieveData, vars);
        ThreadDetach(thread);
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
    return true;
}
bool CMomentumGhostClient::exitGhostClient()
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
unsigned CMomentumGhostClient::sendAndRecieveData(void *params)
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
            steamapicontext->SteamUser()->GetSteamID().ConvertToUint64());

        zed_net_tcp_socket_send(&socket, &newFrame, sizeof(newFrame)); //ACK
    }

    newFrameIdentifier = MOM_S_RECIEVING_NEWFRAME; //Client ready to recieve data from server 
    zed_net_tcp_socket_send(&socket, &newFrameIdentifier, sizeof(newFrameIdentifier)); //SYN

    bytes_read = zed_net_tcp_socket_receive(&socket, &data, sizeof(data));

    if (bytes_read && data == MOM_S_SENDING_NEWFRAME) //SYN-ACK
    {
        //The server then sends number of players to the client
        int playerNum = 0;
        bytes_read = zed_net_tcp_socket_receive(&socket, &playerNum, sizeof(playerNum));

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

    }
    delete vars;
    return 0;
}
static CMomentumGhostClient s_MOMGhostClient;
CMomentumGhostClient *g_pMomentumGhostClient = &s_MOMGhostClient;