#include "cbase.h"
#include "mapzones.h"

#include "tier0/memdbgon.h"

CMapzoneData::CMapzoneData(const char *pMapName)
{
	// Generate file path for zone file
	int pathLength = strlen(mapPath) + strlen(pMapName) + strlen(zoneFileEnding) + 1;
	char* zoneFilePath = new char[pathLength];
	Q_strcpy(zoneFilePath, mapPath);
	Q_strcat(zoneFilePath, pMapName, pathLength);
	Q_strncat(zoneFilePath, zoneFileEnding, pathLength);

	Log("Looking for zone file: ");
	Log(zoneFilePath);
	Log("\n");

	if (g_pFullFileSystem->FileExists(pMapName, "GAME"))
		Log("Found file!\n");

	// TODO: Load zone file and store info in memory
}

CMapzoneData::~CMapzoneData()
{
	// TODO: Remove all previously created triggers
}

void CMapzoneData::SpawnMapZones()
{
	// TODO: Create zone triggers from loaded file info
}