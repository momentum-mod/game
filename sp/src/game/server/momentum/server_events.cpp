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
        // MOM_TODO: connect to site
    }

    void OnMapStart(const char *pMapName)
    {
        // temporary
        if (!Q_strnicmp(pMapName, "surf_", strlen("surf_")))
        {
            g_Timer.SetGameMode(MOMGM_SURF);
        }
        else if(!Q_strnicmp(pMapName, "bhop_", strlen("bhop_")))
        {
            g_Timer.SetGameMode(MOMGM_BHOP);
        }
        else {
            g_Timer.SetGameMode(MOMGM_UNKNOWN);
        }
        
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