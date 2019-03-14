#include "cbase.h"
#include "gameinterface.h"
#include "mapentities.h"
#include "mom_modulecomms.h"

#include "tier0/memdbgon.h"

// -------------------------------------------------------------------------------------------- //
// Mod-specific CServerGameClients implementation.
// -------------------------------------------------------------------------------------------- //

void CServerGameClients::GetPlayerLimits(int& minplayers, int& maxplayers, int &defaultMaxPlayers) const
{
    minplayers = 1;
    maxplayers = 1;
    //defaultMaxPlayers = 1;
}

// -------------------------------------------------------------------------------------------- //
// Mod-specific CServerGameDLL implementation.
// -------------------------------------------------------------------------------------------- //

void CServerGameDLL::LevelInit_ParseAllEntities(const char *pMapEntities)
{
}

void CServerGameDLL::PrepareLevelResources( /* in/out */ char *pszMapName, size_t nMapNameSize,
                                           /* in/out */ char *pszMapFile, size_t nMapFileSize)
{
    g_pModuleComms->FireEvent(new KeyValues("pre_level_init", "map", pszMapName, "file", pszMapFile));
}