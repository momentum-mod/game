#include <winlite.h>
#undef CreateDialog
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#include <time.h>

#include "../../../../../public/vstdlib/pch_vstdlib.h"
#include "../../../../../vgui2/vgui_controls/pch_vgui_controls.h"
#include "../../../../../vgui2/vgui_controls/Frame.h"

#include "tier3/tier3.h"

// steam3 API
//#include "steam/isteammasterserverupdater.h"
//#include "steam/steam_querypackets.h"
#include "../../../../../public/steam/steam_api.h"
#include "../../../../../public/steam/isteamuser.h"
#include "../../../../../public/steam/isteammatchmaking.h"
#include "../../../../../public/steam/isteamfriends.h"

#include "IMapSelector.h"
//#include "ServerBrowser/IServerBrowser.h"
//#include "IVGuiModule.h"
//#include "vgui_controls/Controls.h"

//#include "tier1/netadr.h"
#include "../../../../../public/FileSystem.h"
//#include "iappinformation.h"
//#include "proto_oob.h"
//#include "modlist.h"
//#include "IRunGameEngine.h"
#include "../../../../../public/tier1/KeyValues.h"
#include "../../../../shared/momentum/mom_shareddefs.h"
#include "../../../../shared/momentum/mom_gamerules.h"
#include "../../../../shared/momentum/util/mom_util.h"
#include "../../../../../public/OfflineMode.h"

//VGUI
#include <vgui_controls/pch_vgui_controls.h>

//MapSelection headers
#include "IMapList.h"
#include "MapSelector.h"
#include "MapContextMenu.h"
#include "MapInfoDialog.h"
#include "BaseMapsPage.h"
#include "LocalMaps.h"
#include "OnlineMaps.h"
#include "MapSelectorDialog.h"
#include "cbase.h"
