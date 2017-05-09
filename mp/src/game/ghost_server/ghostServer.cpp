#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <thread>
#include <mutex>

#include "ghostServer.h"
    
void handlePlayer(playerData *newPlayer);
void getInput();
int run_server(unsigned short port);
void new_connection(zed_net_socket_t socket, zed_net_address_t address);
void acceptNewConnections();

volatile int numPlayers = 0;
std::vector<playerData* >m_vecPlayers; 
std::mutex m_vecPlayers_mutex;
std::mutex m_bShouldExit_mutex;
int run_server(unsigned short port)
{
    zed_net_init();

    zed_net_tcp_socket_open(&m_Socket, port, 0, 1);
    printf("Running ghost server on port %d!\n", port);

    return 0;

}
void new_connection(zed_net_socket_t socket, zed_net_address_t address)
{ 
    const char* host;
    int data;
    host = zed_net_host_to_str(address.host);
    printf("Accepted connection from %s:%d\n", host, address.port);

    host = zed_net_host_to_str(address.host);
    int bytes_read = zed_net_tcp_socket_receive(&socket, &data, sizeof(data));
    if (bytes_read)
    {
        printf("Received %d bytes from %s:%d:\n", bytes_read, host, address.port);
        if (data == MOM_SIGNON) //Player signs on for the first time
        {
            printf("Data matches MOM_SIGNON pattern!\n");
            //Describes a new;y connected player with client idx equal to the maximum number of players
            playerData *newPlayer = new playerData(socket, address, numPlayers); 

            m_vecPlayers_mutex.lock();

            m_vecPlayers.push_back(newPlayer);
            numPlayers = m_vecPlayers.size();

            m_vecPlayers_mutex.unlock();

            printf("There are now %i connected players.\n", numPlayers);
            //listen(sock->handle, SOMAXCONN) != 0
            while (newPlayer->remote_socket.ready == 0) //socket is open
            {
                handlePlayer(newPlayer);
            }
            delete newPlayer;
        }
    }
    printf("Thread terminating\n");
    //End of thread
}
void acceptNewConnections()
{
    zed_net_socket_t remote_socket;
    zed_net_address_t remote_address;

    while (!m_bShouldExit)
    {
        zed_net_tcp_accept(&m_Socket, &remote_socket, &remote_address);

        std::thread t(new_connection, remote_socket, remote_address); //create a new thread to deal with the connection
        t.detach(); //each connection is dealt with in a seperate thread
    }
}
void getInput()
{
    char buffer[256];
    char *command, *argument;
    fgets(buffer, 256, stdin);
    buffer[strlen(buffer) - 1] = '\0';
    command = strtok(buffer, " ");
    argument = strtok(NULL, buffer);

    if (strcmp(command, "say") == 0)
    {

    }
    if (strcmp(command, "exit") == 0)
    {
        m_bShouldExit_mutex.lock();
        m_bShouldExit = true;
        m_bShouldExit_mutex.unlock();
    }
    if (strcmp(command, "help") == 0)
    {
        printf("Usage: ghost_server <port> \n");
    }
    if (strcmp(command, "numplayers") == 0)
    {
        printf("Number of connected players: %i\n", numPlayers);
    }
}
void handlePlayer(playerData *newPlayer)
{
    int data;
    int bytes_read = zed_net_tcp_socket_receive(&newPlayer->remote_socket, &data, sizeof(data));
    if (bytes_read && newPlayer)
    {
        if (data == MOM_C_SENDING_NEWFRAME)
        {
            //printf("Data matches MOM_C_SENDING_NEWFRAME pattern! \n"); 
            data = MOM_C_RECIEVING_NEWFRAME;
            zed_net_tcp_socket_send(&newPlayer->remote_socket, &data, sizeof(data)); //SYN-ACK
            // Wait for client to get our acknowledgement, and recieve frame update from client
            zed_net_tcp_socket_receive(&newPlayer->remote_socket, &newPlayer->currentFrame, sizeof(ghostNetFrame)); //ACK
            //printf("Ghost frame: PlayerName: %s\n", newPlayer->currentFrame.PlayerName);
        }

        if (data == MOM_S_RECIEVING_NEWFRAME) //SYN
        {
            //printf("Data matches MOM_S_RECIEVING_NEWFRAME pattern! ..\n");
            // Send out a ACK to the client that we're going to be sending them an update
            data = MOM_S_SENDING_NEWFRAME;
            zed_net_tcp_socket_send(&newPlayer->remote_socket, &data, sizeof(data)); 

            int playerNum = numPlayers;
            //printf("Sending number of players: %i\n", playerNum);
            zed_net_tcp_socket_send(&newPlayer->remote_socket, &playerNum, sizeof(playerNum)); //SYN

            //TODO : Serialize data into one big packet
            /*
            for (int i = 0; i < playerNum; i++)
            {
                //printf("Sending player #%i, name %s ..\n", i, m_vecPlayers[i]->currentFrame.PlayerName);
                zed_net_tcp_socket_send(&newPlayer->remote_socket, &m_vecPlayers[i], sizeof(ghostNetFrame)); //SYN
            }
            */
            

        }
        if (data == MOM_SIGNOFF)
        {
            printf("Data matches MOM_SIGNOFF pattern! Closing socket...\n");
            zed_net_socket_close(&newPlayer->remote_socket);
            m_vecPlayers_mutex.lock();

            m_vecPlayers.erase(m_vecPlayers.begin() + newPlayer->clientIndex);
            numPlayers = m_vecPlayers.size();

            m_vecPlayers_mutex.unlock();
            printf("There are now %i connected players.\n", numPlayers);
            printf("Remote socket status: %i\n", newPlayer->remote_socket.ready);
        }
    }
    //std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
int main(int argc, char** argv)
{
    int status = -1;
    if (argc == 2) 
    {
        status = run_server((unsigned short)atoi(argv[1]));
    }
    else
    {
        status = run_server(DEFAULT_PORT); 
    }
    
    if (status != 0)
        return 0;

    std::thread t(acceptNewConnections); //create a new thread that listens for incoming client connections
    t.detach(); //continuously run thread

    while (!m_bShouldExit)
    {
        getInput();
    }
    zed_net_shutdown();

    return 0;
}