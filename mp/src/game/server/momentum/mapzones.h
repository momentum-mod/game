#ifndef MAPZONES_H
#define MAPZONES_H
#ifdef _WIN32
#pragma once
#endif

// Zone types enum
// NOTE: If adding a new zone type make sure to add a case for it
// in CMapzoneData::LoadFromFile & override ToKeyValues/LoadFromKeyValues
// for the associated trigger class.
enum MomZoneType_t
{
    MOMZONETYPE_INVALID = -1,
    MOMZONETYPE_START = 0,
    MOMZONETYPE_CP,
    MOMZONETYPE_STOP,
    MOMZONETYPE_ONEHOP,
    MOMZONETYPE_RESETONEHOP,
    MOMZONETYPE_CPTELE,
    MOMZONETYPE_MULTIHOP,
    MOMZONETYPE_STAGE,
};

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

#endif
