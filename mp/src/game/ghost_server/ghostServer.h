#include <vector>
#include "mom_shareddefs.h"

//single-file UDP library
#include "zed_net.h"

zed_net_socket_t socket;

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
    frame currentFrame;
    zed_net_socket_t remote_socket;
    zed_net_address_t remote_address;
    playerData(zed_net_socket_t socket, zed_net_address_t addr) 
        : remote_socket(socket), remote_address(addr)
    {
    }
};
std::vector<playerData*>m_vecPlayers;