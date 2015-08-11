#include "filesystem.h"
#include "TimerTriggers.h"

class CMapzone
{
public:
	CMapzone::CMapzone();
	CMapzone::CMapzone(const int, Vector*, QAngle*, Vector*, Vector*, const int);
	~CMapzone();

	void SpawnZone();
	void RemoveZone();

private:
	int m_type; // 0 = start, 1 = checkpoint, 2 = end
	int m_index; // Ignored when not a checkpoint
	Vector* m_pos;
	QAngle* m_rot;
	Vector* m_scaleMins;
	Vector* m_scaleMaxs;
	CBaseEntity* m_trigger;
};

class CMapzoneData
{
public:
	CMapzoneData(const char *szMapName);
	~CMapzoneData();

	void SpawnMapZones();
	void RemoveMapZones();

private:
	const char* c_mapPath = "maps/";
	const char* c_zoneFileEnding = ".zon";

	CUtlVector<CMapzone*> m_zones;
};