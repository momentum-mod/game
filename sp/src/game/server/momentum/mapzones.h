#include "filesystem.h"

class CMapzone
{
public:
	CMapzone::CMapzone();
	CMapzone::CMapzone(const int type, const Vector pos, const Vector rot, const Vector scale);
	CMapzone::CMapzone(const int type, const Vector pos, const Vector rot, const Vector scale, int index);
	~CMapzone();

	void SpawnZone();
	void RemoveZone();

private:
	int type; // 0 = start, 1 = checkpoint, 2 = end
	int index;
	Vector pos;
	Vector rot;
	Vector scale;
};

class CMapzoneData
{
public:
	CMapzoneData(const char *pMapName);
	~CMapzoneData();

	void SpawnMapZones();
	void RemoveMapZones();

private:
	const char* mapPath = "maps/";
	const char* zoneFileEnding = ".zon";

	int zoneCount = 4;
	CMapzone* zones;
};