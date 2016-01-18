#ifndef IMAPLIST_H
#define IMAPLIST_H
#ifdef _WIN32
#pragma once
#endif

class gameserveritem_t;
#if defined( STEAM )
#include "steam2common.h"
#include "FindSteam2Servers.h"
#else
#include "SteamCommon.h"
//#include "FindSteamServers.h"
#endif
//#include "tier1/netadr.h"


typedef enum
{
    SERVERVERSION_SAME_VERSION = 0,
    SERVERVERSION_SERVER_OLD,
    SERVERVERSION_SERVER_NEWER
} SERVERVERSION;

typedef enum
{
    GAMEMODE_UNKNOWN = 0,//Or "All" if the map doesn't have a gametype
    GAMEMODE_SURF,
    GAMEMODE_BHOP
    //MOM_TODO: add more game mode
} GAMEMODE;

//Used by mapdisplay_t, holds map information for filtering and displaying
struct mapstruct_t
{
    char m_szMapName[MAX_PATH];//map name to use for "map m_cMapName"
    int m_iGameMode;//GAMEMODE (Surf/Bhop/KZ/etc)
    bool m_bHasStages;//True if the map has stages
    bool m_bCompleted;//If the player has completed this map or not (read .tim files to set this)
    int m_iDifficulty;//Difficulty of map (Tier 1, 2 etc)
    char m_szBestTime[64];//Best time for the map (MOM_TODO: determine best size for this)
};

//Used by the MapSelectorDialog, encapsulates a map object for the list
struct mapdisplay_t
{
    mapdisplay_t()
    {
        m_iListID = -1;
        m_iServerID = -1;
        m_bDoNotRefresh = true;
    }
    mapstruct_t m_mMap;         // the map struct, containing the information for the map
    int			m_iListID;		// the VGUI2 list panel index for displaying this server
    int			m_iServerID;	// the matchmaking interface index for this server MOM_TODO: remove this
    bool		m_bDoNotRefresh;
    bool operator==(const mapdisplay_t &rhs) const { return rhs.m_iServerID == m_iServerID; }
};

//-----------------------------------------------------------------------------
// Purpose: Interface to accessing a game list
//-----------------------------------------------------------------------------
class IMapList
{
public:

    enum InterfaceItem_e
    {
        FILTERS,
        GETNEWLIST,
        ADDSERVER,
        ADDCURRENTSERVER,
    };

    // returns true if the game list supports the specified ui elements
    virtual bool SupportsItem(InterfaceItem_e item) = 0;

    // starts the servers refreshing
    virtual void StartRefresh() = 0;

    // gets a new server list
    virtual void GetNewServerList() = 0;

    // stops current refresh/GetNewServerList()
    virtual void StopRefresh() = 0;

    // returns true if the list is currently refreshing servers
    virtual bool IsRefreshing() = 0;

    // gets information about specified server
    virtual mapstruct_t *GetMap(unsigned int serverID) = 0;

    // called when Connect button is pressed
    virtual void OnMapStart() = 0;

    // invalid server index
    virtual int GetInvalidServerListID() = 0;
};


#endif // IMAPLIST_H
