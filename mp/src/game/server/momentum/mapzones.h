#pragma once

class CMapzone;

class CMapzoneData
{
public:
    CMapzoneData(const char *szMapName);
    ~CMapzoneData();

    void SpawnMapZones();
    bool LoadFromFile(const char *szMapName);

  private:
    CUtlVector<CMapzone*> m_zones;
};

bool ZoneTypeToClass(int type, char *dest, int maxlen);