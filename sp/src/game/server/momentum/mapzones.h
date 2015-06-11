#include "filesystem.h"

class CMapzoneData
{
public:
	CMapzoneData(const char *pMapName);
	~CMapzoneData();

	void SpawnMapZones();

private:
	const char* mapPath = "maps/";
	const char* zoneFileEnding = ".zon";
};