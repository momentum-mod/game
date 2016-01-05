#include "cbase.h"
#include "movevars_shared.h"
#include "mapzones.h"
#include "Timer.h"
#include "mapzones_edit.h"

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
            sv_maxvelocity.SetValue(3500);
        else
            sv_maxvelocity.SetValue(10000);

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