#include "cbase.h"
#include "movevars_shared.h"

#include "tier0/memdbgon.h"

namespace Momentum {

	void OnClientDLLInit()
	{
		// enable console by default
		ConVarRef con_enable("con_enable");
		con_enable.SetValue(true);
	}

} // namespace Momentum
