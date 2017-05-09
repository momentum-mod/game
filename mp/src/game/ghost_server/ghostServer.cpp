#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <thread>

#include "ghostServer.h"
    
void handlePlayer(playerData *newPlayer);
void getInput();
int numPlayers = 0;
std::vector<playerData* >m_vecPlayers; 
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
        if (data == MOM_SIGNON)
        {
            printf("Data matches MOM_SIGNON pattern!\n");
            playerData *newPlayer = new playerData(socket, address, numPlayers);

            m_vecPlayers.push_back(newPlayer);
            numPlayers = m_vecPlayers.size();
            
            while (socket.ready == 0)
            {
                handlePlayer(newPlayer);
            }
            delete newPlayer;
        }
    }
}
void acceptNewConnections()
{
    zed_net_socket_t remote_socket;
    zed_net_address_t remote_address;
    zed_net_tcp_accept(&m_Socket, &remote_socket, &remote_address);

    std::thread t(new_connection, remote_socket, remote_address); //create a new thread to deal with the connection
    t.detach(); //each connection is dealt with in a seperate thread
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
        m_bShouldExit = true;
    }
    if (strcmp(command, "help") == 0)
    {
        printf("Usage: ghost_server <port> \n");
    }

}
void handlePlayer(playerData *newPlayer)
{
    int data;
    char test[256];
    _snprintf(test, sizeof(test), "Hello! What's up yall?");
    zed_net_tcp_socket_send(&newPlayer->remote_socket, &test, sizeof(test));
    //const char *host;
    //host = zed_net_host_to_str(m_vecPlayers[i]->remote_address.host);
    int bytes_read = zed_net_tcp_socket_receive(&newPlayer->remote_socket, &data, sizeof(data));
    if (bytes_read && newPlayer)
    {
        //printf("Received %d bytes from %s:%d:\n", bytes_read, host, m_vecPlayers[i]->remote_address.port);
        if (data == MOM_SIGNOFF)
        {
            printf("Data matches MOM_SIGNOFF pattern! Closing socket...\n");
            zed_net_socket_close(&newPlayer->remote_socket);
            m_vecPlayers.erase(m_vecPlayers.begin() + newPlayer->clientIndex);
            numPlayers = m_vecPlayers.size();

        }
    }
    //printf("There are %i players connected!\n", numPlayers);
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
        m_bShouldExit = true;

    while (!m_bShouldExit)
    {
        std::thread t(acceptNewConnections); //create a new thread that listens for incoming client connections
        t.join(); //continuously
        //std::thread t2(getInput);
        //t2.detach();

    }
    zed_net_shutdown();

    return 0;
}