#include "cbase.h"
#include "gameinterface.h"
#include "mapentities.h"

#include "tier0/memdbgon.h"

// -------------------------------------------------------------------------------------------- //
// Mod-specific CServerGameClients implementation.
// -------------------------------------------------------------------------------------------- //

void CServerGameClients::GetPlayerLimits(int& minplayers, int& maxplayers, int &defaultMaxPlayers) const
{
    maxplayers = MAX_PLAYERS;
}

// -------------------------------------------------------------------------------------------- //
// Mod-specific CServerGameDLL implementation.
// -------------------------------------------------------------------------------------------- //

void CServerGameDLL::LevelInit_ParseAllEntities(const char *pMapEntities)
{
}