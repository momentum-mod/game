#pragma once

class CMapZone;

class CMapzoneData
{
public:
    CMapzoneData(const char *szMapName);
    ~CMapzoneData();

    void SpawnMapZones();
    bool LoadFromFile(const char *szMapName);

  private:
    CUtlVector<CMapZone*> m_zones;
};

bool ZoneTypeToClass(int type, char *dest, int maxlen);