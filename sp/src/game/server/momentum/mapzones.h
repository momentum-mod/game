#include "filesystem.h"
#include "TimerTriggers.h"

class CMapzone
{
public:
	CMapzone::CMapzone();
	CMapzone::CMapzone(const int, Vector*, QAngle*, Vector*, Vector*, const int,const bool, const float,const int,const bool,const float,const string_t);
	~CMapzone();

	void SpawnZone();
	void RemoveZone();

    int GetType() { return m_type; }
    Vector* GetPosition() { return m_pos; }
    QAngle* GetRotation() { return m_rot; }
    Vector* GetScaleMins() { return m_scaleMins; }
    Vector* GetScaleMaxs() { return m_scaleMaxs; }

private:
	int m_type; // 0 = start, 1 = checkpoint, 2 = end, 3 = Onehop, 4 = OnehopReset, 5 = Checkpoint_teleport, 6 = multihop
	int m_index; // Ignored when not a checkpoint
	bool m_shouldStopOnTeleport; // Stop player on teleport?
	float m_holdTimeBeforeTeleport; // How much to wait for before teleporting
	int m_destinationIndex; // Where to teleport to
	// startTrigger
	bool m_limitingspeed; // Limit leave speed?
	float m_maxleavespeed; // Max speed allowed
	string_t m_linkedtrigger; // Checkpoint name linked to this entity (YESYES, It can be null!)
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
    bool MapZoneSpawned(CMapzone*);
    bool LoadFromFile(const char*);

private:
	const char* c_mapPath = "maps/";
	const char* c_zoneFileEnding = ".zon";

	CUtlVector<CMapzone*> m_zones;
};