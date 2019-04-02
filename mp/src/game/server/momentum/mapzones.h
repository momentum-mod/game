#pragma once

class CMapZone;

class CMapZoneData
{
public:
    CMapZoneData(const char *szMapName);
    ~CMapZoneData();

    void SpawnMapZones();
    bool LoadFromFile(const char *szMapName);

  private:
    CUtlVector<CMapZone*> m_zones;
};

bool ZoneTypeToClass(int type, char *dest, int maxlen);