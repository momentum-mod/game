#pragma once

class CMapZone;

class CMapZoneData
{
public:
    CMapZoneData(const char *szMapName);
    ~CMapZoneData();

    void SpawnMapZones();
    bool LoadFromFile(const char *szMapName);
    bool LoadFromKeyValues(KeyValues *pKvTracks);

  private:
    CUtlVector<CMapZone*> m_Zones;
};

bool ZoneTypeToClass(int type, char *dest, int maxlen);