#pragma once

struct MapData;

class CMapCache : public CAutoGameSystem, public CGameEventListener
{
public:
    CMapCache();

    void OnPlayMap(const char *pMapName);

    void OnPlayerMapLibrary(KeyValues *pKv);

    void FireGameEvent(IGameEvent* event) OVERRIDE;

    MapData *GetCurrentMapData() const { return m_pCurrentMapData; }

protected:
    void PostInit() OVERRIDE;
    void LevelInitPostEntity() OVERRIDE;
    void LevelShutdownPostEntity() OVERRIDE;
    void Shutdown() OVERRIDE;

private:
    MapData *m_pCurrentMapData;

    CUtlDict<uint32> m_dictMapNames;
    CUtlMap<uint32, MapData> m_mapMapCache;
    KeyValues *m_pMapData;
};

extern CMapCache* g_pMapCache;