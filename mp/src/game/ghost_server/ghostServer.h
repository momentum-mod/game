#include <vector>
#include "mom_shareddefs.h"
#include <stdio.h>

//single-file UDP library
#include "zed_net.h"

zed_net_socket_t m_Socket;
bool m_bShouldExit = false;

// Based on CReplayFrame
struct frame
{
    float EyeAngle[3];
    float Position[3];
    float ViewOffset[3];
    int buttons;
    char playerName[MAX_PLAYER_NAME_LENGTH];
};
struct playerData
{
    int clientIndex;
    frame currentFrame;
    zed_net_socket_t remote_socket;
    zed_net_address_t remote_address;
    playerData(zed_net_socket_t socket, zed_net_address_t addr, int idx) 
        : remote_socket(socket), remote_address(addr), clientIndex(idx)
    {
    }
};
