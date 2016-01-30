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
        ConVarRef aa("sv_airaccelerate");
        const char *pMapName = map.GetString();
        // This will only happen if the user didn't use the map selector to start a map

        //set gamemode depending on map name
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
            else if (!Q_strnicmp(pMapName, "kz_", strlen("kz_")))
            {
               DevLog("SETTING THE GAMEMODE!\n");
               gm.SetValue(MOMGM_SCROLL);
            }
            else
            {
                gm.SetValue(MOMGM_UNKNOWN);
                //g_Timer.SetGameMode(MOMGM_UNKNOWN);
            }
        }
        switch (gm.GetInt()) //set aa or other values depending on gamemode 
        {
        case MOMGM_BHOP:
            return aa.SetValue(1000);
            //MOM_TODO : add other possible gm-dependant values to each
        case MOMGM_SCROLL:
            return aa.SetValue(100);

        case MOMGM_SURF:
        case MOMGM_UNKNOWN:
        default:
           return aa.SetValue(150);
        }
    }

    void OnMapStart(const char *pMapName)
    { 
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

        if (!g_Timer.GotCaughtCheating())
        {
            ConVarRef cheatsRef = ConVarRef("sv_cheats");
            if (cheatsRef.GetBool())
                g_Timer.SetCheating(true);
        }
    }

    /*void OnGameFrameEnd()
    {
    }*/

} // namespace Momentum