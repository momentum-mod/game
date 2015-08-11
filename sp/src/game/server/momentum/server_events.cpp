#include "cbase.h"
#include "movevars_shared.h"
#include "mapzones.h"

#include "tier0/memdbgon.h"

namespace Momentum {

	CMapzoneData* zones;

	void OnServerDLLInit()
	{

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
	}

	void OnMapEnd(const char *pMapName)
	{
		// Unload zones
		if (zones)
		{
			delete zones;
			zones = NULL;
		}
	}

} // namespace Momentum