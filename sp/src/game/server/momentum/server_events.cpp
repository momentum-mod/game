#include "cbase.h"
#include "movevars_shared.h"
#include "mapzones.h"
#include "Timer.h"
#include "mapzones_edit.h"
#include "momentum/mom_shareddefs.h"

#include "tier0/memdbgon.h"

namespace Momentum
{

    CMapzoneData* zones;

    void OnServerDLLInit()
    {
        TickSet::TickInit();
        // MOM_TODO: connect to site
    }

    //This is only called when "map ____" is called, if the user uses changelevel then...
    // \/(o_o)\/
    void GameInit()
    {
        ConVarRef gm("mom_gamemode");
        ConVarRef map("host_map");
        const char *pMapName = map.GetString();
        // This will only happen if the user didn't use the map selector to start a map
        if (gm.GetInt() == MOMGM_UNKNOWN)
        {
            if (!Q_strnicmp(pMapName, "surf_", strlen("surf_")))
            {
                gm.SetValue(MOMGM_SURF);
                //g_Timer.SetGameMode(MOMGM_SURF);
            }
            else if (!Q_strnicmp(pMapName, "bhop_", strlen("bhop_")))
            {
                DevLog("SETTING THE GAMEMODE!\n");
                gm.SetValue(MOMGM_BHOP);
                //DevLog("GOT TO #2 %i\n", m_iGameMode);

                //g_Timer.SetGameMode(MOMGM_BHOP);
            }
            else
            {
                gm.SetValue(MOMGM_UNKNOWN);
                //g_Timer.SetGameMode(MOMGM_UNKNOWN);
            }
        }   
    }

    void OnMapStart(const char *pMapName)
    { 
        g_Timer.SetGameModeConVars();

        DevMsg("sv_maxvelocity: %i\n", sv_maxvelocity.GetInt());

        // (Re-)Load zones
        if (zones)
        {
            delete zones;
            zones = NULL;
        }
        zones = new CMapzoneData(pMapName);
        zones->SpawnMapZones();

        //Setup timer
        g_Timer.OnMapStart(pMapName);
        
        // Reset zone editing
        g_MapzoneEdit.Reset();
    }

    void OnMapEnd(const char *pMapName)
    {
        // Unload zones
        if (zones)
        {
            delete zones;
            zones = NULL;
        }

        ConVarRef gm("mom_gamemode");
        gm.SetValue(gm.GetDefault());

        g_Timer.OnMapEnd(pMapName);
    }

    void OnGameFrameStart()
    {
        g_MapzoneEdit.Update();
    }

    /*void OnGameFrameEnd()
    {
    }*/

} // namespace Momentum