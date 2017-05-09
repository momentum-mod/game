#include <vector>
#include "mom_shareddefs.h"
#include <stdio.h>

//single-file UDP library
#include "zed_net.h"

zed_net_socket_t m_Socket;
bool m_bShouldExit = false;
int m_iTickRate;

struct playerData
{
    int clientIndex;
    ghostNetFrame currentFrame;
    zed_net_socket_t remote_socket;
    zed_net_address_t remote_address;
    playerData(zed_net_socket_t socket, zed_net_address_t addr, int idx) 
        : remote_socket(socket), remote_address(addr), clientIndex(idx)
    {
    }
};
