#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ghostServer.h"

int run_server(unsigned short port)
{
    zed_net_init();

    zed_net_tcp_socket_open(&socket, port, 0, 1);
    printf("Running ghost server on port %d!\n", port);

    return 0;

}

int main(int argc, char** argv)
{
    bool should_exit = false;
    int status = -1;
    //char *command, *argument;
    int data;
    if (argc == 2) 
    {
        status = run_server((unsigned short)atoi(argv[1]));
    }
    else
    {
        status = run_server(9000); //default port 9000
    }

    if (status != 0)
        should_exit = true;

    while (!should_exit)
    {
        const char *host;
        zed_net_socket_t remote_socket;
        zed_net_address_t remote_address;

        if (zed_net_tcp_accept(&socket, &remote_socket, &remote_address))
        {
            printf("Failed to accept connection\n");
        }

        host = zed_net_host_to_str(remote_address.host);
        printf("Accepted connection from %s:%d\n", host, remote_address.port);
        playerData *newPlayer = new playerData(remote_socket, remote_address);
        m_vecPlayers.push_back(newPlayer);
        delete newPlayer;

        for (auto i : m_vecPlayers)
        {
            host = zed_net_host_to_str(m_vecPlayers[int(i)]->remote_address.host);
            int bytes_read = zed_net_tcp_socket_receive(&m_vecPlayers[int(i)]->remote_socket, &data, sizeof(data));
            if (bytes_read)
            {
                printf("Received %d bytes from %s:%d:\n", bytes_read, host, m_vecPlayers[int(i)]->remote_address.port);
                if (data == MOM_SIGNON)
                {
                    printf("Data matches MOM_SIGNON pattern!\n");
                }
                if (data == MOM_SIGNOFF)
                {
                    printf("Data matches MOM_SIGNOFF pattern! Closing socket...\n");
                    zed_net_socket_close(&socket);
                }
            }
        }

        /*
        fgets(buffer, 256, stdin);
        buffer[strlen(buffer) - 1] = '\0';
        command = strtok(buffer, " ");
        argument = strtok(NULL, buffer);

        if (strcmp(command, "say") == 0)
        {

        }
        if (strcmp(command, "exit") == 0)
        {
            should_exit = true;
        }
        if (strcmp(command, "help") == 0)
        {
            printf("Usage: ghost_server <port> \n");
        }
        */
    }
    zed_net_shutdown();
    return 0;
}