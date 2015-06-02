#include "cbase.h"
#include "movevars_shared.h"

#include "tier0/memdbgon.h"

namespace Momentum {

void OnServerDLLInit()
{

}

void OnMapStart(const char *pMapName)
{
	// temporary
	if (!Q_strnicmp(pMapName, "surf_", strlen("surf_")))
		sv_maxvelocity.SetValue(3500);
	else
		sv_maxvelocity.SetValue(10000000);

	// do stuff like creating zones here
}

void OnMapEnd(const char *pMapName)
{

}

} // namespace Momentum
